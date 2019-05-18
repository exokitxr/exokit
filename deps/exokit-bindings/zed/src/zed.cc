#include <zed.h>

#include <exout>

// using namespace sl;

namespace zed {

bool cudaSafeCall(cudaError_t err) {
  if (err != cudaSuccess) {
    printf("Cuda error [%d]: %s.\n", err, cudaGetErrorString(err));
    return false;
  }
  return true;
}

Zed::Zed() : tex(0), leftTex(0), rightTex(0), window(nullptr), textureWidth(0), textureHeight(0), /*pcuImageRes(nullptr),*/ leftCudaImageResource(nullptr), rightCudaImageResource(nullptr) {}

Zed::~Zed() {}

NAN_METHOD(Zed::New) {
  Local<Object> zedObj = info.This();
  Zed *zed = new Zed();
  zed->Wrap(zedObj);

  info.GetReturnValue().Set(zedObj);
}

void RunInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  Zed *zed = (Zed *)handle->data;

  Local<Array> result = Nan::New<Array>(zed->chunks.size());

  Local<String> texStr = JS_STR("tex");
  Local<Object> texObj = Nan::New<Object>();
  texObj->Set(JS_STR("id"), JS_INT(zed->tex));

  for (size_t i = 0; i < zed->chunks.size(); i++) {
    size_t chunkIndex = zed->chunks[i];
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

    Local<SharedArrayBuffer> arrayBuffer = SharedArrayBuffer::New(Isolate::GetCurrent(), numPositions*sizeof(float) + numNormals*sizeof(float) + numUvs*sizeof(float) + numIndices*sizeof(uint16_t));
    unsigned char *arrayBufferData = (unsigned char *)arrayBuffer->GetContents().Data();
    int index = 0;

    chunkObj->Set(JS_STR("positionArray"), Float32Array::New(arrayBuffer, index, numPositions));
    memcpy(arrayBufferData + index, positions.data(), numPositions*sizeof(float));
    index += numPositions*sizeof(float);

    chunkObj->Set(JS_STR("normalArray"), Float32Array::New(arrayBuffer, index, numNormals));
    memcpy(arrayBufferData + index, normals.data(), numNormals*sizeof(float));
    index += numNormals*sizeof(float);

    chunkObj->Set(JS_STR("uvArray"), Float32Array::New(arrayBuffer, index, numUvs));
    memcpy(arrayBufferData + index, uvs.data(), numUvs*sizeof(float));
    index += numUvs*sizeof(float);

    chunkObj->Set(JS_STR("indexArray"), Uint16Array::New(arrayBuffer, index, numIndices));
    for (size_t i = 0; i < indices.size(); i++) {
      sl::uint3 &indexVector = indices[i];
      uint16_t *baseIndex = &(((uint16_t *)(arrayBufferData + index))[i*3]);
      baseIndex[0] = (uint16_t)indexVector.x;
      baseIndex[1] = (uint16_t)indexVector.y;
      baseIndex[2] = (uint16_t)indexVector.z;
    }
    index += numUvs*sizeof(uint16_t);

    chunkObj->Set(texStr, texObj);

    result->Set(i, chunkObj);
  }

  zed->result.Reset(result);

  uv_sem_post(&zed->reqSem);
}

NAN_METHOD(Zed::RequestPresent) {
  Zed *zed = ObjectWrap::Unwrap<Zed>(info.This());

  uv_loop_t *loop = windowsystembase::GetEventLoop();
  zed->async = new uv_async_t();
  uv_async_init(loop, zed->async, RunInMainThread);
  zed->async->data = zed;
  uv_sem_init(&zed->reqSem, 0);
  zed->thread = std::thread([zed]() -> void {
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

    sl::CameraParameters camLeft = zed->camera.getCameraInformation().calibration_parameters.left_cam;
    sl::CameraParameters camRight = zed->camera.getCameraInformation().calibration_parameters.right_cam;

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
    // glGenTextures(1, &zed->tex);

    {
      glGenTextures(1, &zed->leftTex);
      glBindTexture(GL_TEXTURE_2D, zed->leftTex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, camLeft.image_size.width, camLeft.image_size.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);

      cudaSafeCall(cudaGraphicsGLRegisterImage(&zed->leftCudaImageResource, zed->leftTex, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard));
      // cudaSafeCall(cudaGraphicsUnregisterResource(zed->leftCudaImageResource));
    }
    {
      glGenTextures(1, &zed->rightTex);
      glBindTexture(GL_TEXTURE_2D, zed->rightTex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, camLeft.image_size.width, camLeft.image_size.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);

      cudaSafeCall(cudaGraphicsGLRegisterImage(&zed->rightCudaImageResource, zed->rightTex, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard));
      // cudaSafeCall(cudaGraphicsUnregisterResource(zed->rightCudaImageResource));
    }

    for (;;) {
      zed->Poll();
    }
  });
  // zed->cbFn.Reset(localCbFn);
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

void Zed::Poll() {
  Zed *zed = this;

  if (zed->camera.grab() == sl::SUCCESS) {
    // Update pose data (used for projection of the mesh over the current image)
    sl::Pose pose;
    sl::TRACKING_STATE tracking_state = zed->camera.getPosition(pose);
    
    // Retrieve image in GPU memory
    zed->camera.retrieveImage(zed->leftImage, sl::VIEW_LEFT, sl::MEM_GPU);
    zed->camera.retrieveImage(zed->rightImage, sl::VIEW_RIGHT, sl::MEM_GPU);

    {
      std::lock_guard<std::mutex> lock(zed->mutex);

      zed->position = pose.getTranslation();
      zed->orientation = pose.getOrientation();
    }

    // Compute elapse time since the last call of Camera::requestMeshAsync()
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - zed->ts_last).count();
    // Ask for a mesh update if 500ms have spend since last request
    if(duration > 500) {
      zed->camera.requestSpatialMapAsync();
      zed->ts_last = std::chrono::high_resolution_clock::now();
    }

    if(zed->camera.getSpatialMapRequestStatusAsync() == sl::SUCCESS) {
      zed->camera.retrieveSpatialMapAsync(zed->mesh);

      sl::MeshFilterParameters filter_params;
      filter_params.set(sl::MeshFilterParameters::MESH_FILTER_MEDIUM);

      // Filter the extracted mesh
      // zed->mesh.filter(filter_params, true);

      // zed->mesh.applyTexture(sl::MESH_TEXTURE_RGB);

      {
        cudaArray_t ArrIm;
        cudaSafeCall(cudaGraphicsMapResources(1, &zed->leftCudaImageResource, 0));
        cudaSafeCall(cudaGraphicsSubResourceGetMappedArray(&ArrIm, zed->leftCudaImageResource, 0, 0));
        cudaSafeCall(cudaMemcpy2DToArray(ArrIm, 0, 0, zed->leftImage.getPtr<sl::uchar1>(sl::MEM_GPU), zed->leftImage.getStepBytes(sl::MEM_GPU), zed->leftImage.getPixelBytes()*zed->leftImage.getWidth(), zed->leftImage.getHeight(), cudaMemcpyDeviceToDevice));
        cudaSafeCall(cudaGraphicsUnmapResources(1, &zed->leftCudaImageResource, 0));
      }
      {
        cudaArray_t ArrIm;
        cudaSafeCall(cudaGraphicsMapResources(1, &zed->rightCudaImageResource, 0));
        cudaSafeCall(cudaGraphicsSubResourceGetMappedArray(&ArrIm, zed->rightCudaImageResource, 0, 0));
        cudaSafeCall(cudaMemcpy2DToArray(ArrIm, 0, 0, zed->rightImage.getPtr<sl::uchar1>(sl::MEM_GPU), zed->rightImage.getStepBytes(sl::MEM_GPU), zed->rightImage.getPixelBytes()*zed->rightImage.getWidth(), zed->rightImage.getHeight(), cudaMemcpyDeviceToDevice));
        cudaSafeCall(cudaGraphicsUnmapResources(1, &zed->rightCudaImageResource, 0));
      }

      /* sl::Texture &texture = zed->mesh.texture;
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
      cudaGraphicsUnmapResources(1, &zed->pcuImageRes, 0); */

      const float range = 5;
      zed->chunks = zed->mesh.getSurroundingList(pose.pose_data, range);

      if (zed->chunks.size() > 0) {
        uv_async_send(zed->async);
        uv_sem_wait(&zed->reqSem);
      }

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

NAN_METHOD(Zed::WaitGetPoses) {
  Zed *zed = ObjectWrap::Unwrap<Zed>(info.This());
  Local<Float32Array> position = Local<Float32Array>::Cast(info[0]);
  Local<Float32Array> orientation = Local<Float32Array>::Cast(info[1]);

  float *positions = (float *)((char *)position->Buffer()->GetContents().Data() + position->ByteOffset());
  float *orientations = (float *)((char *)orientation->Buffer()->GetContents().Data() + orientation->ByteOffset());

  {
    std::lock_guard<std::mutex> lock(zed->mutex);

    memcpy(positions, zed->position.v, 3*sizeof(float));
    memcpy(orientations, zed->orientation.v, 4*sizeof(float));
  }

  if (!zed->result.IsEmpty()) {
    Local<Array> result = Nan::New(zed->result);
    zed->result.Reset();
    info.GetReturnValue().Set(result);
  } else {
    info.GetReturnValue().Set(Nan::Null());
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