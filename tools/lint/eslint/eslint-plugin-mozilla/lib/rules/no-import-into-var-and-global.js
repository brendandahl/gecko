/**
 * @fileoverview Reject use of Cu.import where it attempts to import into
 *               a var and into the global scope, e.g.
 *               var foo = Cu.import("path.jsm", this);
 *
 *               This is considered bad practice as it is confusing as to what
 *               is actually being imported.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

"use strict";

// -----------------------------------------------------------------------------
// Rule Definition
// -----------------------------------------------------------------------------

var helpers = require("../helpers");

module.exports = function(context) {

  // ---------------------------------------------------------------------------
  // Public
  //  --------------------------------------------------------------------------

  return {
    "CallExpression": function(node) {
      if (node.callee.type === "MemberExpression" &&
          node.parent.type === "VariableDeclarator" &&
          node.arguments.length === 2) {
        let memexp = node.callee;
        if (((memexp.object.type === "Identifier" &&
              memexp.object.name === "Cu") ||
             (memexp.object.type === "MemberExpression" &&
              memexp.object.object && memexp.object.property &&
              memexp.object.object.name === "Components" &&
              memexp.object.property.name === "utils")) &&
            memexp.property.type === "Identifier" &&
            memexp.property.name === "import" &&
            node.arguments[1].type === "ThisExpression") {
          context.report(node, "Cu.import imports into variables and into " +
                         "global scope.");
        }
      }
    }
  };
};
