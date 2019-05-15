#include <zed.h>

#include <exout>

// using namespace sl;

namespace zed {

Zed::Zed() : tex(0), window(nullptr), textureWidth(0), textureHeight(0), pcuImageRes(nullptr) {}

Zed::~Zed() {}

NAN_METHOD(Zed::New) {
  Nan::HandleScope scope;

  Local<Object> zedObj = info.This();
  Zed *zed = new Zed();
  zed->Wrap(zedObj);

  info.GetReturnValue().Set(zedObj);
}

NAN_METHOD(Zed::RequestPresent) {
  Zed *zed = ObjectWrap::Unwrap<Zed>(info.This());
  Local<Function> localCbFn = Local<Function>::Cast(info[0]);

  // Setup configuration parameters for the ZED    
  sl::InitParameters parameters;
  parameters.coordinate_units = sl::UNIT_METER;
  parameters.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP; // OpenGL coordinates system

  // Open the ZED
  sl::ERROR_CODE zed_error = zed->camera.open(parameters);
  if(zed_error != sl::ERROR_CODE::SUCCESS) {
      std::cout << zed_error << std::endl;
      zed->camera.close();
      return;
  }

  sl::CameraParameters camera_parameters = zed->camera.getCameraInformation().calibration_parameters.left_cam;

  // sl::Mat image; // current left image

  sl::SpatialMappingParameters spatial_mapping_parameters;
  // sl::SPATIAL_MAPPING_STATE mapping_state = sl::SPATIAL_MAPPING_STATE_NOT_ENABLED;
  // bool mapping_activated = false; // indicates if the spatial mapping is running or not
  zed->ts_last = std::chrono::high_resolution_clock::now(); // time stamp of the last mesh request

  // Enable positional tracking before starting spatial mapping
  zed->camera.enableTracking();

  {
    sl::Transform init_pose;
    zed->camera.resetTracking(init_pose);

    // Configure Spatial Mapping parameters
    spatial_mapping_parameters.resolution_meter = sl::SpatialMappingParameters::get(sl::SpatialMappingParameters::MAPPING_RESOLUTION_MEDIUM);
    spatial_mapping_parameters.use_chunk_only = true;
    spatial_mapping_parameters.save_texture = true;
    spatial_mapping_parameters.map_type = sl::SpatialMappingParameters::SPATIAL_MAP_TYPE_MESH;				
    // Enable spatial mapping
    try {
        zed->camera.enableSpatialMapping(spatial_mapping_parameters);
        std::cout << "Spatial Mapping will output a " << spatial_mapping_parameters.map_type << std::endl;
    } catch(std::string e) {
        std::cout <<"Error enabling Spatial Mapping "<< e << std::endl;
    }
  }

  zed->window = windowsystem::CreateWindowHandle(1, 1, false);
  windowsystem::SetCurrentWindowContext(zed->window);
  glGenTextures(1, &zed->tex);
  zed->cbFn.Reset(localCbFn);
  // cudaError_t cudaError = cudaGraphicsGLRegisterImage(&zed->pcuImageRes, zed->tex, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);
  // cudaError = cudaGraphicsUnregisterResource(zed->pcuImageRes);
}

NAN_METHOD(Zed::ExitPresent) {
  Zed *zed = ObjectWrap::Unwrap<Zed>(info.This());

  // image.free();
  zed->mesh.clear();

  zed->camera.disableSpatialMapping();
  zed->camera.disableTracking();
  zed->camera.close();
}

NAN_METHOD(Zed::WaitGetPoses) {
  Zed *zed = ObjectWrap::Unwrap<Zed>(info.This());
  
  if (zed->camera.grab() == sl::SUCCESS) {
    // Retrieve image in GPU memory
    // zed.retrieveImage(image, sl::VIEW_LEFT, sl::MEM_GPU);
    // Update pose data (used for projection of the mesh over the current image)
    sl::Pose pose; // positional tracking data
    sl::TRACKING_STATE tracking_state = zed->camera.getPosition(pose);

    // Compute elapse time since the last call of Camera::requestMeshAsync()
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - zed->ts_last).count();
    // Ask for a mesh update if 500ms have spend since last request
    if(duration > 2000) {
      zed->camera.requestSpatialMapAsync();
      zed->ts_last = std::chrono::high_resolution_clock::now();
    }

  if(zed->camera.getSpatialMapRequestStatusAsync() == sl::SUCCESS) {
      zed->camera.retrieveSpatialMapAsync(zed->mesh);

      sl::MeshFilterParameters filter_params;
      filter_params.set(sl::MeshFilterParameters::MESH_FILTER_MEDIUM);
      // Filter the extracted mesh
      zed->mesh.filter(filter_params, true);

      zed->mesh.applyTexture(sl::MESH_TEXTURE_RGB);

      sl::Texture &texture = zed->mesh.texture;
      sl::Mat &textureMaterial = texture.data;
      // sl::Resolution &textureResolution = textureMaterial.getResolution();
      // sl::uchar1 *tex = textureMaterial.getPtr<sl::uchar1>(sl::MEM_GPU);

      size_t width = textureMaterial.getWidth();
      size_t height = textureMaterial.getHeight();
      if (width != zed->textureWidth || height != zed->textureHeight) {
        if (zed->pcuImageRes) {
          cudaError_t cudaError = cudaGraphicsUnregisterResource(zed->pcuImageRes);
          zed->pcuImageRes = nullptr;
        }
        
        windowsystem::SetCurrentWindowContext(zed->window);

        glBindTexture(GL_TEXTURE_2D, zed->tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);

        cudaError_t cudaError = cudaGraphicsGLRegisterImage(&zed->pcuImageRes, zed->tex, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);

        zed->textureWidth = width;
        zed->textureHeight = height;
      }

      cudaArray_t arrIm;
      cudaGraphicsMapResources(1, &zed->pcuImageRes, 0);
      cudaGraphicsSubResourceGetMappedArray(&arrIm, zed->pcuImageRes, 0, 0);
      cudaMemcpy2DToArray(arrIm, 0, 0, textureMaterial.getPtr<sl::uchar1>(sl::MEM_GPU), textureMaterial.getStepBytes(sl::MEM_GPU), width * sizeof(sl::uchar3), height, cudaMemcpyDeviceToDevice);
      cudaGraphicsUnmapResources(1, &zed->pcuImageRes, 0);
      
      Local<Object> asyncObject = Nan::New<Object>();
      AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "Zed::WaitGetPoses");

      Local<Function> localCbFn = Nan::New(zed->cbFn);
      const float range = 5;
      sl::Mesh::chunkList chunks = zed->mesh.getSurroundingList(pose.pose_data, range);
      Local<Value> result;
      if (chunks.size() > 0) {
        Local<Array> array = Nan::New<Array>(chunks.size());

        Local<String> texStr = JS_STR("tex");
        Local<Object> texObj = Nan::New<Object>();
        texObj->Set(JS_STR("id"), JS_INT(zed->tex));

        for (size_t i = 0; i < chunks.size(); i++) {
          size_t chunkIndex = chunks[i];
          sl::Chunk &chunk = zed->mesh[chunkIndex];
          const sl::float3 &position = chunk.barycenter;
          std::vector<sl::float3> &positions = chunk.vertices;
          size_t numPositions = positions.size() * sizeof(positions[0]) / sizeof(float);
          std::vector<sl::float3> &normals = chunk.normals;
          size_t numNormals = normals.size() * sizeof(normals[0]) / sizeof(float);
          std::vector<sl::float2> &uvs = chunk.uv;
          size_t numUvs = uvs.size() * sizeof(uvs[0]) / sizeof(float);
          std::vector<sl::uint3> &indices = chunk.triangles;
          size_t numIndices = indices.size() * sizeof(indices[0]) / sizeof(uint32_t);

          Local<Object> chunkObj = Nan::New<Object>();
          chunkObj->Set(JS_STR("positionArray"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), positions[0].v, numPositions * sizeof(float)), 0, numPositions));
          chunkObj->Set(JS_STR("normalArray"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), normals[0].v, numNormals * sizeof(float)), 0, numNormals));
          chunkObj->Set(JS_STR("uvArray"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), uvs[0].v, numUvs * sizeof(float)), 0, numUvs));
          chunkObj->Set(JS_STR("indexArray"), Uint32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), indices[0].v, numIndices * sizeof(uint32_t)), 0, numIndices));
          chunkObj->Set(texStr, texObj);
          array->Set(i, chunkObj);
        }
        
        result = array;
      } else {
        result = Nan::Null();
      }
      Local<Value> argv[] = {
        result,
      };
      asyncResource.MakeCallback(localCbFn, sizeof(argv)/sizeof(argv[0]), argv);
      
      /* //Save as an OBJ file
      bool error_save = zed->mesh.save("C:\\Users\\avaer\\Documents\\GitHub\\exokit\\mesh_gen.obj");
      if(error_save) {
        std::cout << ">> Mesh saved" << std::endl;
      } else {
        std::cout << ">> Failed to save mesh" << std::endl;
      } */
    }
  }
}

Local<Object> Zed::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Zed"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "RequestPresent", RequestPresent);
  Nan::SetMethod(proto, "ExitPresent", ExitPresent);
  Nan::SetMethod(proto, "WaitGetPoses", WaitGetPoses);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

  return scope.Escape(ctorFn);
}

}

Local<Object> makeZed() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  Local<Object> exports = zed::Zed::Initialize(isolate);

  return scope.Escape(exports);
}