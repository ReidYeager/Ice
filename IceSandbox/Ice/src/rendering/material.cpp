
#include "defines.h"
#include "logger.h"

#include "rendering/renderer.h"
#include "tools/lexer.h"

#include <vector>
#include <fstream>
#include <iostream>

u32 reIceRenderer::CreateMaterial(const std::vector<IceShader>& _shaders)
{
  IceMaterial newMaterial;
  newMaterial.shaderIndices.resize(_shaders.size());
  u32 matIndex = 0;
  u32 maxBindingIndex = 0;

  // Get shaders =====
  for (u32 i = 0; i < _shaders.size(); i++)
  {
    newMaterial.shaderIndices[i] = GetShader(_shaders[i].directory, _shaders[i].stage);

    if (shaders[i].descriptors.size() > 0)
    {
      u32 shaderMaxBindingIndex = shaders[i].descriptors[shaders[i].descriptors.size() - 1].bindingIndex;
      if (shaderMaxBindingIndex > maxBindingIndex)
      {
        maxBindingIndex = shaderMaxBindingIndex;
      }
    }
  }

  // TODO : ~!!~ Get descriptors with the lexer
  /*
  * Need to get proper descriptor parsing up & running first

  // Get descriptors =====
  newMaterial.bindings.resize(maxBindingIndex);
  // TODO : create empty descriptors for unused bindings

  std::vector<IceShaderBinding>& mBinds = newMaterial.bindings;

  for (const auto& sIdx : newMaterial.shaderIndices)
  {
    for (const auto& desc : shaders[sIdx].descriptors)
    {
      //if (newMaterial.bindings[desc.bindingIndex].descriptor.type != desc.type)
      if (mBinds[desc.bindingIndex].descriptor.bindingIndex == 255)
      {
        // Add the new binding
        mBinds[desc.bindingIndex].descriptor = desc;
      }
      else if (mBinds[desc.bindingIndex].descriptor.type != desc.type)
      {
        IceLogFatal("Conflicting shader binding %u : %u -- %u",
                    desc.bindingIndex, mBinds[desc.bindingIndex].descriptor.type, desc.type);
        return false;
      }
    }
  }
  */

  std::vector<IceHandle> backendShaderIndices;
  for (const IceHandle& s : newMaterial.shaderIndices)
  {
    backendShaderIndices.push_back(shaders[s].backendShader);
  }

  // Need to sync front and backend shaders (Init the lighting material with this)
  return backend.CreateMaterial(backendShaderIndices);
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

    IceShader newShader = { fullDir, _stage, -1, {} };

    newShader.backendShader = backend.CreateShader(_directory, _stage);

    // Get descriptors =====
    // Store descriptors sorted by binding index
    GetShaderDescriptors(newShader);

    // Create backend assets =====
    // backend.CreateShader(newShader.descriptors);
    shaders.push_back(newShader);
  }

  return index;
}

void reIceRenderer::GetShaderDescriptors(IceShader& _shader)
{
  IceLogError("Renderer descriptor parsing not yet implemented.");

  std::string descDir = _shader.directory;
  descDir.append(".desc");

  std::fstream descFile(descDir.c_str());
}
