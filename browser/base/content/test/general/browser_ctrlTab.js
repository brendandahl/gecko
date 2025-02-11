add_task(function* () {
  gPrefService.setBoolPref("browser.ctrlTab.previews", true);

  gBrowser.addTab();
  gBrowser.addTab();
  gBrowser.addTab();

  checkTabs(4);

  yield ctrlTabTest([2], 1, 0);
  yield ctrlTabTest([2, 3, 1], 2, 2);
  yield ctrlTabTest([], 4, 2);

  {
    let selectedIndex = gBrowser.tabContainer.selectedIndex;
    yield pressCtrlTab();
    yield pressCtrlTab(true);
    yield releaseCtrl();
    is(gBrowser.tabContainer.selectedIndex, selectedIndex,
       "Ctrl+Tab -> Ctrl+Shift+Tab keeps the selected tab");
  }

  { // test for bug 445369
    let tabs = gBrowser.tabs.length;
    yield pressCtrlTab();
    yield synthesizeCtrlW();
    is(gBrowser.tabs.length, tabs - 1, "Ctrl+Tab -> Ctrl+W removes one tab");
    yield releaseCtrl();
  }

  { // test for bug 667314
    let tabs = gBrowser.tabs.length;
    yield pressCtrlTab();
    yield pressCtrlTab(true);
    yield synthesizeCtrlW();
    is(gBrowser.tabs.length, tabs - 1, "Ctrl+Tab -> Ctrl+W removes the selected tab");
    yield releaseCtrl();
  }

  gBrowser.addTab();
  checkTabs(3);
  yield ctrlTabTest([2, 1, 0], 7, 1);

  { // test for bug 1292049
    let tabToClose = yield BrowserTestUtils.openNewForegroundTab(gBrowser, "about:buildconfig");
    checkTabs(4);
    selectTabs([0, 1, 2, 3]);

    yield BrowserTestUtils.removeTab(tabToClose);
    checkTabs(3);
    undoCloseTab();
    checkTabs(4);
    is(gBrowser.tabContainer.selectedIndex, 3, "tab is selected after closing and restoring it");

    yield ctrlTabTest([], 1, 2);
  }

  { // test for bug 445369
    checkTabs(4);
    selectTabs([1, 2, 0]);

    let selectedTab = gBrowser.selectedTab;
    let tabToRemove = gBrowser.tabs[1];

    yield pressCtrlTab();
    yield pressCtrlTab();
    yield synthesizeCtrlW();
    ok(!tabToRemove.parentNode,
       "Ctrl+Tab*2 -> Ctrl+W removes the second most recently selected tab");

    yield pressCtrlTab(true);
    yield pressCtrlTab(true);
    yield releaseCtrl();
    ok(selectedTab.selected,
       "Ctrl+Tab*2 -> Ctrl+W -> Ctrl+Shift+Tab*2 keeps the selected tab");
  }
  gBrowser.removeTab(gBrowser.tabContainer.lastChild);
  checkTabs(2);

  yield ctrlTabTest([1], 1, 0);

  gBrowser.removeTab(gBrowser.tabContainer.lastChild);
  checkTabs(1);

  { // test for bug 445768
    let focusedWindow = document.commandDispatcher.focusedWindow;
    let eventConsumed = true;
    let detectKeyEvent = function(event) {
      eventConsumed = event.defaultPrevented;
    };
    document.addEventListener("keypress", detectKeyEvent, false);
    yield pressCtrlTab();
    document.removeEventListener("keypress", detectKeyEvent, false);
    ok(eventConsumed, "Ctrl+Tab consumed by the tabbed browser if one tab is open");
    is(focusedWindow, document.commandDispatcher.focusedWindow,
       "Ctrl+Tab doesn't change focus if one tab is open");
  }

  // cleanup
  if (gPrefService.prefHasUserValue("browser.ctrlTab.previews"))
    gPrefService.clearUserPref("browser.ctrlTab.previews");

  /* private utility functions */

  function* pressCtrlTab(aShiftKey) {
    let promise;
    if (!isOpen() && canOpen()) {
      promise = BrowserTestUtils.waitForEvent(ctrlTab.panel, "popupshown");
    } else {
      promise = BrowserTestUtils.waitForEvent(document, "keyup");
    }
    EventUtils.synthesizeKey("VK_TAB", { ctrlKey: true, shiftKey: !!aShiftKey });
    return promise;
  }

  function* releaseCtrl() {
    let promise;
    if (isOpen()) {
      promise = BrowserTestUtils.waitForEvent(ctrlTab.panel, "popuphidden");
    } else {
      promise = BrowserTestUtils.waitForEvent(document, "keyup");
    }
    EventUtils.synthesizeKey("VK_CONTROL", { type: "keyup" });
    return promise;
  }

  function* synthesizeCtrlW() {
    let promise = BrowserTestUtils.waitForEvent(gBrowser.tabContainer, "TabClose");
    EventUtils.synthesizeKey("w", { ctrlKey: true });
    return promise;
  }

  function isOpen() {
    return ctrlTab.isOpen;
  }

  function canOpen() {
    return gPrefService.getBoolPref("browser.ctrlTab.previews") && gBrowser.tabs.length > 2;
  }

  function checkTabs(aTabs) {
    is(gBrowser.tabs.length, aTabs, "number of open tabs should be " + aTabs);
  }

  function selectTabs(tabs) {
    tabs.forEach(function(index) {
      gBrowser.selectedTab = gBrowser.tabs[index];
    });
  }

  function* ctrlTabTest(tabsToSelect, tabTimes, expectedIndex) {
    selectTabs(tabsToSelect);

    var indexStart = gBrowser.tabContainer.selectedIndex;
    var tabCount = gBrowser.tabs.length;
    var normalized = tabTimes % tabCount;
    var where = normalized == 1 ? "back to the previously selected tab" :
                normalized + " tabs back in most-recently-selected order";

    for (let i = 0; i < tabTimes; i++) {
      yield pressCtrlTab();

      if (tabCount > 2)
       is(gBrowser.tabContainer.selectedIndex, indexStart,
         "Selected tab doesn't change while tabbing");
    }

    if (tabCount > 2) {
      ok(isOpen(),
         "With " + tabCount + " tabs open, Ctrl+Tab opens the preview panel");

      yield releaseCtrl();

      ok(!isOpen(),
         "Releasing Ctrl closes the preview panel");
    } else {
      ok(!isOpen(),
         "With " + tabCount + " tabs open, Ctrl+Tab doesn't open the preview panel");
    }

    is(gBrowser.tabContainer.selectedIndex, expectedIndex,
       "With " + tabCount + " tabs open and tab " + indexStart
       + " selected, Ctrl+Tab*" + tabTimes + " goes " + where);
  }
});
