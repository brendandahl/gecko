/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

const { classes: Cc, interfaces: Ci, results: Cr, utils: Cu } = Components;
Cu.import("resource://gre/modules/Services.jsm");

const appShell = Cc["@mozilla.org/appshell/appShellService;1"].getService(Ci.nsIAppShellService);
this.EXPORTED_SYMBOLS = ["Runtime"];

this.Runtime = {
  start: function() {
    Services.startup.enterLastWindowClosingSurvivalArea();
    let windowlessBrowser = appShell.createWindowlessBrowser(true);
    this.windowlessBrowser = windowlessBrowser;
    var webNavigation = windowlessBrowser.QueryInterface(Ci.nsIWebNavigation);
    webNavigation.loadURI("chrome://headless/content/bootstrap.html", Ci.nsIWebNavigation.LOAD_FLAGS_NONE, null, null, null);
  },
  stop: function() {
    this.windowlessBrowser.close();
    this.windowlessBrowser = null;
    Services.startup.exitLastWindowClosingSurvivalArea();
  }
};
