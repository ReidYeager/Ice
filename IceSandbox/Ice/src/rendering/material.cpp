
#include "defines.h"
#include "logger.h"

#include "platform/file_system.h"
#include "rendering/renderer.h"
#include "tools/lexer.h"

#include <vector>
#include <fstream>
#include <iostream>

u32 reIceRenderer::CreateMaterial(const std::vector<IceShader>& _shaders, u32 _subpassIndex)
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
    newMaterial.shaderIndices[i] = GetShader(_shaders[i].directory, _shaders[i].stage);
  }

  // Get descriptors =====
  std::vector<IceShaderBinding>& mBinds = newMaterial.bindings;

  for (const auto& sIdx : newMaterial.shaderIndices)
  {
    for (const auto& desc : shaders[sIdx].descriptors)
    {
      mBinds.push_back({desc, nullptr});
    }
  }

  std::vector<IceHandle> backendShaderIndices;
  for (const IceHandle& s : newMaterial.shaderIndices)
  {
    backendShaderIndices.push_back(shaders[s].backendShader);
  }

  // Need to sync front and backend shaders (Init the lighting material with this)
  u32 index = materials.size();
  newMaterial.backendMaterial = backend.CreateMaterial(backendShaderIndices,
                                                       newMaterial.bindings,
                                                       _subpassIndex);
  materials.push_back(newMaterial);

  return index;
}

u32 reIceRenderer::GetShader(const std::string& _directory, IceShaderStage _stage)
{
  u32 index = 0;

  // Look for existing shader =====
  for (const auto& s : shaders)
  {
    if (s.directory.compare(_directory.c_str()) == 0 && s.stage & _stage)
    {
      return index;
    }
    index++;
  }

  // Create new shader =====
  {
    std::string fullDir = ICE_RESOURCE_SHADER_DIR;
    fullDir.append(_directory);

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
    default: IceLogError("Shader stage %u is unsupported", _stage); return -1;
    }

    IceShader newShader = { fullDir, _stage, -1, {} };

    newShader.backendShader = backend.CreateShader(fullDir, _stage);

    // Get descriptors =====
    // Store descriptors sorted by binding index
    ICE_ATTEMPT(GetShaderDescriptors(newShader));

    // Create backend assets =====
    // backend.CreateShader(newShader.descriptors);
    shaders.push_back(newShader);
  }

  return index;
}

b8 reIceRenderer::GetShaderDescriptors(IceShader& _shader)
{
  std::string descDir = _shader.directory;
  descDir.append(".desc");

  std::vector<char> descriptorSource = fileSystem.LoadFile(descDir.c_str());

  if (descriptorSource.size() == 0)
    return true; // No descriptors to look for

  IceLexer lexer(descriptorSource.data());
  IceLexerToken token;

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

      _shader.bufferParameterIndices.push_back(bufferParameterIndex);
    }

  }

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

      IceShaderDescriptor newDesc = { (u8)_shader.descriptors.size(),
                                      (IceShaderDescriptorType)descTypeIndex };
      _shader.descriptors.push_back(newDesc);
    }
  }

  return true;
}

void reIceRenderer::ReloadMaterials()
{
  for (auto& s : shaders)
  {
    backend.RecreateShader(s);
    s.descriptors.clear();
    GetShaderDescriptors(s);
  }

  for (auto& m : materials)
  {
    // Get descriptors =====
    for (const auto& shaderIndex : m.shaderIndices)
    {
      for (const auto& desc : shaders[shaderIndex].descriptors)
      {
        m.bindings.push_back({desc, nullptr});
      }
    }

    std::vector<IceHandle> backendShaderIndices;
    for (const IceHandle& s : m.shaderIndices)
    {
      backendShaderIndices.push_back(shaders[s].backendShader);
    }

    backend.RecreateMaterial(m.backendMaterial, backendShaderIndices, m.bindings);
  }
}
