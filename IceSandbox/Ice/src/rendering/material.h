
#ifndef ICE_RENDERING_MATERIAL_H_
#define ICE_RENDERING_MATERIAL_H_

#include "defines.h"
#include "logger.h"

#include "rendering/render_context.h"

#include <vector>

std::vector<IceShader> shaders;
std::vector<IceMaterial> materials;

void GetShader(std::string _directory, IceShaderStage _stage)
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
    IceShader newShader = { _directory, _stage, 0, {} };

    // Get descriptors =====
    // Store descriptors sorted by binding index
    // TODO : ~!!~
    newShader.descriptors = {};

    // Create backend assets =====
    // backend.CreateShader(newShader.descriptors);
    shaders.push_back(newShader);
  }

  return index;
}

b8 GetMaterial(std::vector<IceShader> _shaders)
{
  IceMaterial newMaterial;
  newMaterial.shaderIndices.resize(_shaders.size());
  u32 matIndex = 0;
  u32 maxBindingIndex = 0;

  // Get shaders =====
  for (u32 i = 0; i < _shaders.size(); i++)
  {
    newMaterial.shaderIndices[i] = GetShader(_shaders[i].directory, _shaders[i].stage);

    u32 shaderMaxBindingIndex = shaders[i].descriptors[shaders[i].descriptors.size() - 1].bindingIndex;
    if (shaderMaxBindingIndex > maxBindingIndex)
    {
      maxBindingIndex = shaderMaxBindingIndex;
    }
  }

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

  return true;
}

#endif // !define ICE_RENDERING_MATERIAL_H_
