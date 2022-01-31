
#include "defines.h"
#include "logger.h"

#include "platform/file_system.h"
#include "rendering/renderer.h"
#include "tools/lexer.h"

#include <vector>
#include <fstream>
#include <iostream>

IceHandle IceRenderer::CreateMaterial(const std::vector<IceShader>& _shaders, u32 _subpassIndex)
{
  #ifdef ICE_DEBUG
  {
    std::string materialDebugString = "Creating material with {";
    for (const auto& s : _shaders)
    {
      materialDebugString.append(s.directory);
      switch (s.stage)
      {
      case Ice_Shader_Vertex: materialDebugString.append(".vert"); break;
      case Ice_Shader_Fragment: materialDebugString.append(".frag"); break;
      }
      materialDebugString.append(", ");
    }
    materialDebugString.erase(materialDebugString.size() - 2, 2);
    materialDebugString.append("}");
    IceLogInfo(materialDebugString.c_str());
  }
  #endif // ICE_DEBUG

  IceMaterial newMaterial;
  newMaterial.shaderIndices.resize(_shaders.size());
  u32 matIndex = 0;

  // Get shaders =====
  for (u32 i = 0; i < _shaders.size(); i++)
  {
    IceHandle nextShader = GetShader(_shaders[i].directory, _shaders[i].stage);
    if (nextShader == ICE_NULL_HANDLE)
    {
      IceLogError("Material attempting to use invalid shader : '%s' in stage %u",
                  _shaders[i].directory.c_str(), _shaders[i].stage);
      return ICE_NULL_HANDLE;
    }

    newMaterial.shaderIndices[i] = nextShader;
  }

  // Get descriptors =====
  std::vector<IceShaderDescriptor>& mBinds = newMaterial.descriptors;

  for (const auto& sIdx : newMaterial.shaderIndices)
  {
    for (const auto& desc : shaders[sIdx].descriptors)
    {
      mBinds.push_back(desc);
    }
  }

  std::vector<IceHandle> backendShaderIndices;
  for (const IceHandle& s : newMaterial.shaderIndices)
  {
    backendShaderIndices.push_back(shaders[s].backendShader);
  }

  newMaterial.backendMaterial = backend.CreateNewMaterial(backendShaderIndices,
                                                          newMaterial.descriptors,
                                                          _subpassIndex);
  materials.push_back(newMaterial);

  return materials.size() - 1;
}

u32 IceRenderer::GetShader(const std::string& _directory, IceShaderStage _stage)
{
  u32 index = 0;

  std::string fullDir = ICE_RESOURCE_SHADER_DIR;
  fullDir.append(_directory);

  // Look for existing shader =====
  for (const auto& s : shaders)
  {
    if (s.directory.compare(fullDir.c_str()) == 0 && s.stage & _stage)
    {
      return index;
    }
    index++;
  }

  // Create new shader =====
  {
    IceShader newShader = { fullDir, _stage, ICE_NULL_HANDLE, {} };

    switch (_stage)
    {
    case Ice_Shader_Vertex:
    {
      fullDir.append(".vert");
    } break;
    case Ice_Shader_Fragment:
    {
      fullDir.append(".frag");
    } break;
    default: IceLogError("Shader stage %u is unsupported", _stage); return ICE_NULL_HANDLE;
    }

    newShader.backendShader = backend.CreateShader(fullDir, _stage);
    if (newShader.backendShader == ICE_NULL_HANDLE)
      return ICE_NULL_HANDLE;

    // Get descriptors =====
    // Store descriptors sorted by binding index
    ICE_ATTEMPT_BOOL(GetShaderDescriptors(newShader));

    // Create backend assets =====
    // backend.CreateShader(newShader.descriptors);
    shaders.push_back(newShader);
  }

  return index;
}

b8 IceRenderer::GetShaderDescriptors(IceShader& _shader)
{
  std::string descDir = _shader.directory;
  switch (_shader.stage)
  {
  case Ice_Shader_Vertex:
  {
    descDir.append(".vert");
  } break;
  case Ice_Shader_Fragment:
  {
    descDir.append(".frag");
  } break;
  }
  descDir.append(".desc");

  std::vector<char> descriptorSource = fileSystem.LoadFile(descDir.c_str());

  if (descriptorSource.size() == 0)
    return true; // No descriptors to look for

  IceLexer lexer(descriptorSource.data());
  IceLexerToken token;

  // Buffer info =====
  u32 bufferParameterCount = 0;
  if (lexer.CheckForExpectedToken("buffer") && lexer.ExpectToken("{"))
  {
    while (!lexer.CheckForExpectedToken("}"))
    {
      token = lexer.NextToken();

      u32 bufferParameterIndex = -1;

      for (u32 i = 0; i < Ice_Shader_Buffer_Count; i++)
      {
        if (token.string.compare(IceShaderBufferInputNames[i]) == 0)
        {
          bufferParameterIndex = i;
          break;
        }
      }

      if (bufferParameterIndex == -1)
      {
        IceLogError("%s contains invalid buffer parameter '%s'",
                    descDir.c_str(), token.string.c_str());
        continue;
      }

      _shader.bufferParameters |= (1 << bufferParameterIndex);
      bufferParameterCount++;
    }

  }

  // Bindings =====
  // TODO : Define descriptor bind index, Define sampler image default value? (normal, white, black)
  if (lexer.CheckForExpectedToken("bindings") && lexer.ExpectToken("{"))
  {
    while (!lexer.CheckForExpectedToken("}"))
    {
      token = lexer.NextToken();

      u32 descTypeIndex = -1;

      for (u32 i = 0; i < Ice_Descriptor_Type_Count; i++)
      {
        if (token.string.compare(IceDescriptorTypeNames[i]) == 0)
        {
          descTypeIndex = i;
          break;
        }
      }

      if (descTypeIndex == -1)
      {
        IceLogError("%s contains invalid descriptor '%s'",
                    descDir.c_str(), token.string.c_str());
        continue;
      }

      IceShaderDescriptor newDesc = { (IceShaderDescriptorType)descTypeIndex,
                                      (u8)_shader.descriptors.size(),
                                      0,
                                      ICE_NULL_HANDLE};

      if (descTypeIndex == Ice_Descriptor_Type_Buffer)
      {
        newDesc.data = bufferParameterCount * 16;
      }

      _shader.descriptors.push_back(newDesc);
    }
  }

  return true;
}

// TODO : Rework descriptors/bindings to account for reloading materials
void IceRenderer::ReloadMaterials()
{
  for (auto& s : shaders)
  {
    s.descriptors.clear();
    GetShaderDescriptors(s);
    backend.RecreateShader(s);
  }

  for (auto& m : materials)
  {
    // Save the descriptors =====
    std::vector<IceShaderDescriptor> bufferStack;
    std::vector<IceShaderDescriptor> samplerStack;
    for (i32 i = m.descriptors.size() - 1; i >= 0; i--)
    {
      IceShaderDescriptor d = m.descriptors[i];

      switch (d.type)
      {
      case Ice_Descriptor_Type_Buffer : bufferStack.push_back(d); break;
      case Ice_Descriptor_Type_Sampler2D : samplerStack.push_back(d); break;
      default: break;
      }
    }

    i32 bsEnd = bufferStack.size() - 1;
    i32 ssEnd = samplerStack.size() - 1;

    // Reuse the material's old descriptors =====
    m.descriptors.clear();
    for (auto& sIdx : m.shaderIndices)
    {
      for (auto& desc : shaders[sIdx].descriptors)
      {
        // Attempt to reuse a descriptor =====
        switch (desc.type)
        {
        case Ice_Descriptor_Type_Buffer:
        {
          if (bsEnd < 0)
            break;

          m.descriptors.push_back(bufferStack[bsEnd]);
          bsEnd--;
        } continue;
        case Ice_Descriptor_Type_Sampler2D:
        {
          if (ssEnd < 0)
            break;

          m.descriptors.push_back(samplerStack[ssEnd]);
          ssEnd--;
        } continue;
        default: break;
        }

        // Use the new descriptor =====
        m.descriptors.push_back(desc);
      }
    }

    std::vector<IceHandle> backendShaderIndices;
    for (const IceHandle& s : m.shaderIndices)
    {
      backendShaderIndices.push_back(shaders[s].backendShader);
    }

    backend.RecreateMaterial(m.backendMaterial, backendShaderIndices, m.descriptors);
  }
}

b8 IceRenderer::SetMaterialBufferData(IceHandle _material, void* _data)
{
  u32 bindingIndex = -1;

  IceMaterial& mat = materials[_material];

  for (u32 i = 0; i < mat.descriptors.size(); i++)
  {
    if (mat.descriptors[i].type == Ice_Descriptor_Type_Buffer)
    {
      bindingIndex = mat.descriptors[i].backendHandle;
      break;
    }
  }

  if (bindingIndex == -1)
    return false;

  ICE_ATTEMPT_BOOL(backend.FillMaterialBuffer(bindingIndex, _data));

  return true;
}
