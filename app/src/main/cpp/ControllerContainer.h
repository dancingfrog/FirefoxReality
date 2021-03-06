/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VRBROWSER_CONTROLLER_CONTAINER_H
#define VRBROWSER_CONTROLLER_CONTAINER_H

#include "ControllerDelegate.h"
#include "Controller.h"

#include "vrb/Forward.h"
#include "vrb/MacroUtils.h"
#include "vrb/Matrix.h"
#include <memory>
#include <string>
#include <vector>

namespace crow {

class ControllerContainer;
typedef std::shared_ptr<ControllerContainer> ControllerContainerPtr;

class ControllerContainer : public crow::ControllerDelegate  {
public:
  enum class HandEnum { Left, Right };
  static ControllerContainerPtr Create(vrb::CreationContextPtr& aContext, const vrb::GroupPtr& aPointerContainer);
  vrb::TogglePtr GetRoot() const;
  void LoadControllerModel(const int32_t aModelIndex, const vrb::ModelLoaderAndroidPtr& aLoader, const std::string& aFileName);
  void InitializeBeam();
  void Reset();
  std::vector<Controller>& GetControllers();
  const std::vector<Controller>& GetControllers() const;
  // crow::ControllerDelegate interface
  void CreateController(const int32_t aControllerIndex, const int32_t aModelIndex, const std::string& aImmersiveName) override;
  void DestroyController(const int32_t aControllerIndex) override;
  void SetEnabled(const int32_t aControllerIndex, const bool aEnabled) override;
  void SetVisible(const int32_t aControllerIndex, const bool aVisible) override;
  void SetTransform(const int32_t aControllerIndex, const vrb::Matrix& aTransform) override;
  void SetButtonCount(const int32_t aControllerIndex, const uint32_t aNumButtons) override;
  void SetButtonState(const int32_t aControllerIndex, const Button aWhichButton, const int32_t aImmersiveIndex, const bool aPressed, const bool aTouched, const float aImmersiveTrigger = -1.0f) override;
  void SetAxes(const int32_t aControllerIndex, const float* aData, const uint32_t aLength) override;
  void SetLeftHanded(const int32_t aControllerIndex, const bool aLeftHanded) override;
  void SetTouchPosition(const int32_t aControllerIndex, const float aTouchX, const float aTouchY) override;
  void EndTouch(const int32_t aControllerIndex) override;
  void SetScrolledDelta(const int32_t aControllerIndex, const float aScrollDeltaX, const float aScrollDeltaY) override;
  void SetPointerColor(const vrb::Color& color) const;
  void SetVisible(const bool aVisible);
protected:
  struct State;
  ControllerContainer(State& aState, vrb::CreationContextPtr& aContext);
  ~ControllerContainer();
private:
  State& m;
  ControllerContainer() = delete;
  VRB_NO_DEFAULTS(ControllerContainer)
};

} // namespace crow

#endif //VRBROWSER_CONTROLLER_CONTAINER_H
