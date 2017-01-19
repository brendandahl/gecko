/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const { classes: Cc, interfaces: Ci, results: Cr, utils: Cu } = Components;

Cu.import("resource:///modules/Runtime.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");

function CommandLineHandler() {}

CommandLineHandler.prototype = {
  classID: Components.ID("{236b79c3-ab58-446f-abba-4caba4deb337}"),

  /* nsISupports */

  QueryInterface: XPCOMUtils.generateQI([Ci.nsICommandLineHandler]),

  /* nsICommandLineHandler */

  helpInfo: "",

  handle: function(cmdLine) {
    Runtime.start();
  },
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([CommandLineHandler]);
