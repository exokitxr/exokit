#include <zed.h>

#include <exout>

// using namespace sl;

namespace zed {

Zed::Zed() {}

Zed::~Zed() {}

NAN_METHOD(Zed::New) {
  Nan::HandleScope scope;

  Local<Object> zedObj = info.This();
  Zed *zed = new Zed();
  zed->Wrap(zedObj);

  info.GetReturnValue().Set(zedObj);
}

NAN_METHOD(Zed::RequestPresent) {
  sl::Camera zed;
  // Setup configuration parameters for the ZED    
  sl::InitParameters parameters;
  parameters.coordinate_units = sl::UNIT_METER;
  parameters.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP; // OpenGL coordinates system

  // Open the ZED
  sl::ERROR_CODE zed_error = zed.open(parameters);
  if(zed_error != sl::ERROR_CODE::SUCCESS) {
      std::cout << zed_error << std::endl;
      zed.close();
      return;
  }

  sl::CameraParameters camera_parameters = zed.getCameraInformation().calibration_parameters.left_cam;

  // sl::Mat image; // current left image
  sl::Pose pose; // positional tracking data

  sl::Mesh map; // current incemental mesh

  sl::SpatialMappingParameters spatial_mapping_parameters;
  sl::TRACKING_STATE tracking_state = sl::TRACKING_STATE_OFF;
  sl::SPATIAL_MAPPING_STATE mapping_state = sl::SPATIAL_MAPPING_STATE_NOT_ENABLED;
  bool mapping_activated = false; // indicates if the spatial mapping is running or not
  std::chrono::high_resolution_clock::time_point ts_last = std::chrono::high_resolution_clock::now(); // time stamp of the last mesh request
  
  // Enable positional tracking before starting spatial mapping
  zed.enableTracking();
  
  {
    sl::Transform init_pose;
    zed.resetTracking(init_pose);

    // Configure Spatial Mapping parameters
    spatial_mapping_parameters.resolution_meter = sl::SpatialMappingParameters::get(sl::SpatialMappingParameters::MAPPING_RESOLUTION_MEDIUM);
    spatial_mapping_parameters.use_chunk_only = true;
    spatial_mapping_parameters.save_texture = true;
    spatial_mapping_parameters.map_type = sl::SpatialMappingParameters::SPATIAL_MAP_TYPE_MESH;				
    // Enable spatial mapping
    try {
        zed.enableSpatialMapping(spatial_mapping_parameters);
        std::cout << "Spatial Mapping will output a " << spatial_mapping_parameters.map_type << std::endl;
    } catch(std::string e) {
        std::cout <<"Error enable Spatial Mapping "<< e << std::endl;
    }
  }

  for (;;) {
      if(zed.grab() == sl::SUCCESS) {
          // Retrieve image in GPU memory
          // zed.retrieveImage(image, sl::VIEW_LEFT, sl::MEM_GPU);
          // Update pose data (used for projection of the mesh over the current image)
          tracking_state = zed.getPosition(pose);

          // Compute elapse time since the last call of Camera::requestMeshAsync()
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - ts_last).count();
          // Ask for a mesh update if 500ms have spend since last request
          if(duration > 2000) {
              zed.requestSpatialMapAsync();
              ts_last = std::chrono::high_resolution_clock::now();
          }

          if(zed.getSpatialMapRequestStatusAsync() == sl::SUCCESS) {
              zed.retrieveSpatialMapAsync(map);
              
              sl::MeshFilterParameters filter_params;
              filter_params.set(sl::MeshFilterParameters::MESH_FILTER_MEDIUM);
              // Filter the extracted mesh
              map.filter(filter_params, true);
              
              map.applyTexture(sl::MESH_TEXTURE_RGB);
              
              sl::Texture &texture = map.texture;
              sl::Mat &textureMaterial = texture.data;
              sl::Resolution &textureResolution = textureMaterial.getResolution();
              sl::uchar1 *tex = textureMaterial.getPtr<sl::uchar1>(sl::MEM_GPU);
              
              sl::Mesh::chunkList chunks = map.getSurroundingList(pose.pose_data, 5);
              for (auto iter = chunks.begin(); iter != chunks.end(); iter++) {
                auto chunkIndex = *iter;
                sl::Chunk &chunk = map[chunkIndex];
                const sl::float3 &position = chunk.barycenter;
                std::vector<sl::float3> &positions = chunk.vertices;
                std::vector<sl::float3> &normals = chunk.normals;
                std::vector<sl::float2> &uvs = chunk.uv;
                std::vector<sl::uint3> &indices = chunk.triangles;
              }
              
              //Save as an OBJ file
              bool error_save = map.save("C:\\Users\\avaer\\Documents\\GitHub\\exokit\\mesh_gen.obj");
              if(error_save) {
                std::cout << ">> Mesh saved" << std::endl;
              } else {
                std::cout << ">> Failed to save mesh" << std::endl;
              }
              
              break;
          }
      }
  }

  // image.free();
  map.clear();

  zed.disableSpatialMapping();
  zed.disableTracking();
  zed.close();
}

NAN_METHOD(Zed::WaitGetPoses) {
  // XXX
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