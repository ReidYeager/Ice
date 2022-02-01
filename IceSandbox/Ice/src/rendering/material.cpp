
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

  // Shaders =====
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

  // Descriptors =====
  ICE_ATTEMPT_BOOL_HANDLE(PopulateMaterialDescriptors(&newMaterial));

  std::vector<IceHandle> backendShaderIndices;
  for (const IceHandle& s : newMaterial.shaderIndices)
  {
    backendShaderIndices.push_back(shaders[s].backendShader);
  }

  // Create =====
  newMaterial.backendMaterial = backend.CreateNewMaterial(backendShaderIndices,
                                                          newMaterial.descriptors,
                                                          _subpassIndex);
  materials.push_back(newMaterial);

  return materials.size() - 1;
}

b8 IceRenderer::PopulateMaterialDescriptors(IceMaterial* _material,
    std::unordered_map<IceShaderDescriptorType, std::vector<IceShaderDescriptor>>* _stacks)
{
  std::vector<IceShaderDescriptor>& matDescs = _material->descriptors;
  matDescs.clear();

  for (const auto& shaderIndex : _material->shaderIndices)
  {
    for (const auto& descriptor : shaders[shaderIndex].descriptors)
    {
      u32 bindIndex = descriptor.bindingIndex;

      if (bindIndex >= matDescs.size())
      {
        matDescs.resize(bindIndex + 1);

        // Not my favorite workaround for handling gaps
        u32 tmpIndex = 0;
        for (auto& tmpd : matDescs)
        {
          if (tmpd.bindingIndex == 255)
            tmpd.bindingIndex = tmpIndex;
          tmpIndex++;
        }
      }

      if (matDescs[bindIndex].type == Ice_Descriptor_Type_Invalid)
      {
        // Reuse a descriptor
        if (_stacks != nullptr)
        {
          auto& stack = _stacks->at(descriptor.type);

          if (stack.size() != 0)
          {
            matDescs[bindIndex] = stack[stack.size() - 1];
            matDescs[bindIndex].bindingIndex = bindIndex;
            stack.pop_back();
            continue;
          }
        }

        // Add a new descriptor
        matDescs[bindIndex] = descriptor;
      }
      else if (matDescs[bindIndex].type != descriptor.type)
      {
        IceLogFatal("Descriptor type conflict : Binding %u in '%s'\n\t%s vs %s -- Keeping %s",
          bindIndex,
          shaders[shaderIndex].directory.c_str(),
          IceDescriptorTypeNames[matDescs[bindIndex].type],
          IceDescriptorTypeNames[descriptor.type],
          IceDescriptorTypeNames[matDescs[bindIndex].type]);
        return false;
      }
    }
  }

  return true;
}

IceHandle IceRenderer::GetShader(const std::string& _directory, IceShaderStage _stage)
{
  u32 index = 0;

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
  default: IceLogError("Shader stage %u is unsupported", _stage); return ICE_NULL_HANDLE;
  }

  // Look for existing shader =====
  for (const auto& s : shaders)
  {
    if (s.directory.compare(fullDir.c_str()) == 0)
    {
      return index;
    }
    index++;
  }

  // Create new shader =====
  {
    IceShader newShader = { fullDir, _stage, ICE_NULL_HANDLE, {} };

    newShader.backendShader = backend.CreateShader(fullDir, _stage);
    if (newShader.backendShader == ICE_NULL_HANDLE)
      return ICE_NULL_HANDLE;

    // Get descriptors =====
    // Store descriptors sorted by binding index
    ICE_ATTEMPT_BOOL_HANDLE(GetShaderDescriptors(newShader));

    // Create backend assets =====
    // backend.CreateShader(newShader.descriptors);
    shaders.push_back(newShader);
  }

  return index;
}

b8 IceRenderer::GetShaderDescriptors(IceShader& _shader)
{
  std::string descDir = _shader.directory;
  descDir.append(".desc");

  std::vector<char> descriptorSource = fileSystem.LoadFile(descDir.c_str());

  if (descriptorSource.size() == 0)
    return true; // No descriptors to look for

  IceLexer lexer(descriptorSource.data());
  IceLexerToken token;

  i32 maxBindingIndex = -1;

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

      // Additional descriptor info =====
      u32 bindingIndex = 255;

      while (lexer.CheckForExpectedToken(","))
      {
        token = lexer.NextToken();
        switch (token.type)
        {
        case Ice_Token_Int:
        case Ice_Token_Float:
        {
          bindingIndex = lexer.UintFromToken(&token);
        } break;
        default: break;
        }
      }

      if (bindingIndex == 255)
      {
        return false;
      }

      IceShaderDescriptor newDesc = { (IceShaderDescriptorType)descTypeIndex,
                                      (u8)bindingIndex,
                                      0,
                                      ICE_NULL_HANDLE};

      if (bindingIndex > maxBindingIndex)
      {
        maxBindingIndex = bindingIndex;
      }

      if (descTypeIndex == Ice_Descriptor_Type_Buffer)
      {
        newDesc.data = bufferParameterCount * 16;
      }

      _shader.descriptors.push_back(newDesc);
    }
  }

  return true;
}

b8 IceRenderer::ReloadMaterials()
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
    std::unordered_map<IceShaderDescriptorType, std::vector<IceShaderDescriptor>> stacks;

    for (i32 i = m.descriptors.size() - 1; i >= 0; i--)
    {
      IceShaderDescriptor d = m.descriptors[i];
      stacks[d.type].push_back(d);
    }

    // Recreate material =====
    ICE_ATTEMPT_BOOL(PopulateMaterialDescriptors(&m, &stacks));

    std::vector<IceHandle> backendShaderIndices;
    for (const IceHandle& s : m.shaderIndices)
    {
      backendShaderIndices.push_back(shaders[s].backendShader);
    }

    ICE_ATTEMPT_BOOL(backend.RecreateMaterial(m.backendMaterial,
                                              backendShaderIndices,
                                              m.descriptors));

  }

  return true;
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
