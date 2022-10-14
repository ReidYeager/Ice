
#ifndef ICE_CORE_APPLICATION_H_
#define ICE_CORE_APPLICATION_H_

#include "defines.h"
#include "core/application_defines.h"

#include "core/platform/platform.h"
#include "rendering/renderer.h"
#include "core/ecs/ecs.h"
#include "tools/array.h"

namespace Ice {

//=========================
// Application
//=========================

u32 CreateVersion(u8 _major, u8 _minor, u8 _patch);

u32 Run(ApplicationSettings);

// End the application loop peacefully
void Shutdown();

//=========================
// Platform
//=========================

void CloseWindow();

//void* AllocateMemory(u64 _size);
//void FreeMemory(void* _data);

//=========================
// Rendering
//=========================

b8 GetMesh(const char* _directory, u32* _mesh);

b8 CreateMaterial(Ice::MaterialSettings _settings, u32* _material = nullptr);
void SetMaterialData(Ice::Material* _material, Ice::BufferSegment _segment, void* _data);

// Reload one shader
b8 ReloadShader(Ice::Shader* _shader);
b8 ReloadAllShaders();

// Reload material's shaders and recreate the material
b8 ReloadMaterial(Ice::Material* _material);
b8 ReloadAllMaterials();
// Only recreate the material -- Does not affect its shaders
b8 RecreateMaterial(Ice::Material* _material);
b8 RecreateAllMaterials();

b8 LoadTexture(Ice::Image* _texture, const char* _directory);
void SetTexture(Ice::Material* _material, u32 _inputIndex, const char* _image);
//void SetTexture(Ice::Material* _material, u32 inputIndex, Ice::Image* _image);

Ice::Entity CreateCamera(Ice::CameraSettings _settings = {});
Ice::Entity CreateRenderedEntity(const char* _meshDir = nullptr,
                                 u32 _material = Ice::null32);

b8 UpdateTransforms();

} // namespace Ice

#endif // !ICE_CORE_APPLICATION_H_
