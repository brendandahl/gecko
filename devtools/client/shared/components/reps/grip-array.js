/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* vim: set ft=javascript ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

// Make this available to both AMD and CJS environments
define(function (require, exports, module) {
  // Dependencies
  const React = require("devtools/client/shared/vendor/react");
  const {
    createFactories,
    isGrip,
    wrapRender,
  } = require("./rep-utils");
  const { Caption } = createFactories(require("./caption"));
  const { MODE } = require("./constants");

  // Shortcuts
  const { span } = React.DOM;

  /**
   * Renders an array. The array is enclosed by left and right bracket
   * and the max number of rendered items depends on the current mode.
   */
  let GripArray = React.createClass({
    displayName: "GripArray",

    propTypes: {
      object: React.PropTypes.object.isRequired,
      // @TODO Change this to Object.values once it's supported in Node's version of V8
      mode: React.PropTypes.oneOf(Object.keys(MODE).map(key => MODE[key])),
      provider: React.PropTypes.object,
    },

    getLength: function (grip) {
      if (!grip.preview) {
        return 0;
      }

      return grip.preview.length || grip.preview.childNodesLength || 0;
    },

    getTitle: function (object, context) {
      let objectLink = this.props.objectLink || span;
      if (this.props.mode !== MODE.TINY) {
        return objectLink({
          object: object
        }, object.class + " ");
      }
      return "";
    },

    getPreviewItems: function (grip) {
      if (!grip.preview) {
        return null;
      }

      return grip.preview.items || grip.preview.childNodes || null;
    },

    arrayIterator: function (grip, max) {
      let items = [];
      const gripLength = this.getLength(grip);

      if (!gripLength) {
        return items;
      }

      const previewItems = this.getPreviewItems(grip);
      if (!previewItems) {
        return items;
      }

      let delim;
      // number of grip preview items is limited to 10, but we may have more
      // items in grip-array.
      let delimMax = gripLength > previewItems.length ?
        previewItems.length : previewItems.length - 1;
      let provider = this.props.provider;

      for (let i = 0; i < previewItems.length && i < max; i++) {
        try {
          let itemGrip = previewItems[i];
          let value = provider ? provider.getValue(itemGrip) : itemGrip;

          delim = (i == delimMax ? "" : ", ");

          items.push(GripArrayItem(Object.assign({}, this.props, {
            object: value,
            delim: delim
          })));
        } catch (exc) {
          items.push(GripArrayItem(Object.assign({}, this.props, {
            object: exc,
            delim: delim
          })));
        }
      }
      if (previewItems.length > max || gripLength > previewItems.length) {
        let objectLink = this.props.objectLink || span;
        let leftItemNum = gripLength - max > 0 ?
          gripLength - max : gripLength - previewItems.length;
        items.push(Caption({
          object: objectLink({
            object: this.props.object
          }, leftItemNum + " more…")
        }));
      }

      return items;
    },

    render: wrapRender(function () {
      let {
        object,
        mode = MODE.SHORT
      } = this.props;

      let items;
      let brackets;
      let needSpace = function (space) {
        return space ? { left: "[ ", right: " ]"} : { left: "[", right: "]"};
      };

      if (mode === MODE.TINY) {
        let objectLength = this.getLength(object);
        let isEmpty = objectLength === 0;
        items = [span({className: "length"}, isEmpty ? "" : objectLength)];
        brackets = needSpace(false);
      } else {
        let max = (mode === MODE.SHORT) ? 3 : 10;
        items = this.arrayIterator(object, max);
        brackets = needSpace(items.length > 0);
      }

      let objectLink = this.props.objectLink || span;
      let title = this.getTitle(object);

      return (
        span({
          className: "objectBox objectBox-array"},
          title,
          objectLink({
            className: "arrayLeftBracket",
            object: object
          }, brackets.left),
          ...items,
          objectLink({
            className: "arrayRightBracket",
            object: object
          }, brackets.right),
          span({
            className: "arrayProperties",
            role: "group"}
          )
        )
      );
    }),
  });

  /**
   * Renders array item. Individual values are separated by
   * a delimiter (a comma by default).
   */
  let GripArrayItem = React.createFactory(React.createClass({
    displayName: "GripArrayItem",

    propTypes: {
      delim: React.PropTypes.string,
    },

    render: function () {
      let { Rep } = createFactories(require("./rep"));

      return (
        span({},
          Rep(Object.assign({}, this.props, {
            mode: MODE.TINY
          })),
          this.props.delim
        )
      );
    }
  }));

  function supportsObject(grip, type) {
    if (!isGrip(grip)) {
      return false;
    }

    return (grip.preview && (
        grip.preview.kind == "ArrayLike" ||
        type === "DocumentFragment"
      )
    );
  }

  // Exports from this module
  exports.GripArray = {
    rep: GripArray,
    supportsObject: supportsObject
  };
});
