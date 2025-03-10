/*
 * Copyright (C) 2021 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import RenderWindowOverlay 1.0
import IgnGazebo 1.0 as IgnGazebo

Rectangle {
  visible: false
  color: "transparent"

  RenderWindowOverlay {
    id: renderWindowOverlay
    objectName: "renderWindowOverlay"
    anchors.fill: parent

    Connections {
      target: renderWindowOverlay
      onOpenContextMenu:
      {
        entityContextMenu.open(_entity, "model",
          _mouseX, _mouseY);
      }
    }
  }

  IgnGazebo.EntityContextMenu {
    id: entityContextMenu
    anchors.fill: parent
  }
}
