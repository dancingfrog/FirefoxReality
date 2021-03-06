/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Skybox.h"
#include "VRLayer.h"
#include "VRLayerNode.h"
#include "vrb/ConcreteClass.h"
#include "vrb/Color.h"
#include "vrb/CreationContext.h"
#include "vrb/Geometry.h"
#include "vrb/Matrix.h"
#include "vrb/ModelLoaderAndroid.h"
#include "vrb/RenderState.h"
#include "vrb/RenderContext.h"
#include "vrb/TextureCubeMap.h"
#include "vrb/Toggle.h"
#include "vrb/Transform.h"
#include "vrb/VertexArray.h"

#include <array>

using namespace vrb;

namespace crow {

static TextureCubeMapPtr LoadTextureCube(vrb::CreationContextPtr& aContext, const std::string& aBasePath,
                                         const std::string& aExtension, GLuint targetTexture = 0) {
  TextureCubeMapPtr cubemap = vrb::TextureCubeMap::Create(aContext, targetTexture);
  cubemap->SetTextureParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  cubemap->SetTextureParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  cubemap->SetTextureParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  cubemap->SetTextureParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  cubemap->SetTextureParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  auto path = [&](const std::string &name) { return aBasePath + "/" + name + aExtension; };
  vrb::TextureCubeMap::Load(aContext, cubemap, path("posx"), path("negx"), path("posy"),
                            path("negy"), path("posz"), path("negz"));
  return cubemap;
}

struct Skybox::State {
  vrb::CreationContextWeak context;
  vrb::TogglePtr root;
  VRLayerCubePtr layer;
  GLuint layerTextureHandle;
  vrb::TransformPtr transform;
  vrb::GeometryPtr geometry;
  vrb::ModelLoaderAndroidPtr loader;
  std::string basePath;
  std::string extension;
  TextureCubeMapPtr texture;
  State():
      layerTextureHandle(0)
  {}

  void Initialize() {
    vrb::CreationContextPtr create = context.lock();
    root = vrb::Toggle::Create(create);
    transform = vrb::Transform::Create(create);
    if (layer) {
      root->AddNode(VRLayerNode::Create(create, layer));
      layer->SetSurfaceChangedDelegate([=](const VRLayer& aLayer, VRLayer::SurfaceChange aChange, const std::function<void()>& aCallback) {
        this->layerTextureHandle = layer->GetTextureHandle();
        LoadLayer();
        if (aCallback) {
          aCallback();
        }
      });
    } else {
      root->AddNode(transform);
    }
  }

  void LoadGeometry() {
    LoadTask task = [=](CreationContextPtr &aContext) -> GroupPtr {
      std::array<GLfloat, 24> cubeVertices{
          -1.0f, 1.0f, 1.0f, // 0
          -1.0f, -1.0f, 1.0f, // 1
          1.0f, -1.0f, 1.0f, // 2
          1.0f, 1.0f, 1.0f, // 3
          -1.0f, 1.0f, -1.0f, // 4
          -1.0f, -1.0f, -1.0f, // 5
          1.0f, -1.0f, -1.0f, // 6
          1.0f, 1.0f, -1.0f, // 7
      };

      std::array<GLushort, 24> cubeIndices{
          0, 1, 2, 3,
          3, 2, 6, 7,
          7, 6, 5, 4,
          4, 5, 1, 0,
          0, 3, 7, 4,
          1, 5, 6, 2
      };

      VertexArrayPtr array = VertexArray::Create(aContext);
      const float kLength = 140.0f;
      for (int i = 0; i < cubeVertices.size(); i += 3) {
        array->AppendVertex(Vector(-kLength * cubeVertices[i], -kLength * cubeVertices[i + 1],
                                   -kLength * cubeVertices[i + 2]));
        array->AppendUV(Vector(-kLength * cubeVertices[i], -kLength * cubeVertices[i + 1],
                               -kLength * cubeVertices[i + 2]));
      }

      if (!geometry) {
        geometry = vrb::Geometry::Create(aContext);
        geometry->SetVertexArray(array);

        for (int i = 0; i < cubeIndices.size(); i += 4) {
          std::vector<int> indices = {cubeIndices[i] + 1, cubeIndices[i + 1] + 1,
                                      cubeIndices[i + 2] + 1, cubeIndices[i + 3] + 1};
          geometry->AddFace(indices, indices, {});
        }
        RenderStatePtr state = RenderState::Create(aContext);
        geometry->SetRenderState(state);
      }


      texture = LoadTextureCube(aContext, basePath, extension);
      vrb::RenderStatePtr state = geometry->GetRenderState();
      state->SetTexture(texture);
      state->SetMaterial(Color(1.0f, 1.0f, 1.0f), Color(1.0f, 1.0f, 1.0f), Color(0.0f, 0.0f, 0.0f),
                         0.0f);
      geometry->SetRenderState(state);
      vrb::GroupPtr group = vrb::Transform::Create(aContext);
      group->AddNode(geometry);
      return group;
    };

    loader->RunLoadTask(transform, task);
  }

  void LoadLayer() {
    if (basePath.empty() || layerTextureHandle == 0) {
      return;
    }
    vrb::CreationContextPtr create = context.lock();
    texture = LoadTextureCube(create, basePath, extension, layerTextureHandle);
    texture->Bind();
    layer->SetLoaded(true);
  }
};

void
Skybox::Load(const vrb::ModelLoaderAndroidPtr& aLoader, const std::string& aBasePath, const std::string& aExtension) {
  if (m.basePath == aBasePath) {
    return;
  }
  m.loader = aLoader;
  m.basePath = aBasePath;
  m.extension = aExtension;
  if (m.layer) {
    m.LoadLayer();
  } else {
    m.LoadGeometry();
  }
}

void
Skybox::SetVisible(bool aVisible) {
  m.root->ToggleAll(aVisible);
}

void
Skybox::SetTransform(const vrb::Matrix& aTransform) {
  if (m.transform) {
    m.transform->SetTransform(aTransform);
  }
}

void
Skybox::SetTintColor(const vrb::Color &aTintColor) {
  if (m.layer) {
    m.layer->SetTintColor(aTintColor);
  } else if (m.geometry) {
    m.geometry->GetRenderState()->SetTintColor(aTintColor);
  }

}

vrb::NodePtr
Skybox::GetRoot() const {
  return m.root;
}


SkyboxPtr
Skybox::Create(vrb::CreationContextPtr aContext, const VRLayerCubePtr& aLayer) {
  SkyboxPtr result = std::make_shared<vrb::ConcreteClass<Skybox, Skybox::State> >(aContext);
  result->m.layer = aLayer;
  result->m.Initialize();
  return result;
}


Skybox::Skybox(State& aState, vrb::CreationContextPtr& aContext) : m(aState) {
  m.context = aContext;
}

Skybox::~Skybox() {}

} // namespace crow
