"use strict";
const device = require("../lib/Device.js");
const Util = require("../lib/Util.js");

const PROTOCOL_TEMPLATE =
{
  "s":[
    { "t":72, "d":120.0, "c":1, "i":5 },
    { "t":84, "d":30.0, "c":1, "i":5 }
  ],
  "h":20
};

class AppState {
  constructor () {
    this.protocols = [];
    this.selectedProtocol = null;
    this.navigationHandler = null;
    
    /* Event handlers */
    this.panelContainer = null;
    this.panelStack = [];
    
    this.PANELS = {
      DASHBOARD: 1,
      PROTOCOL_LIST: 2,
      PROTOCOL_DETAIL: 3,
      PROTOCOL_EDITOR: 4,
      EXPERIMENT_EDITOR: 5,
      EXPERIMENT_LIST: 6,
      EXPERIMENT_MONITOR: 7,
      OPTICS_MONITOR: 8,
      PLATE_CONTROL_MONITOR: 9,
      TEMPLATE: 10
    };
    this.views = {
      protocolDetail: null,
      protocolEditor: null,
      experimentEditor: null,
      experimentMonitor: null
    };
    this.panelStack.push(this.PANELS.DASHBOARD);
  }
  isKiosk () {
    return this.isKiosk = location.href.indexOf("kiosk=true")>0;
  }
  setNavigationHandler (handler) {
    this.navigationHandler = handler;
  }
  getPanel (panel) {
    return this.viewsMap[panel];
  }
  setViews (views) {
    this.views = views;
    this.viewsMap = [];
    this.viewsMap[this.PANELS.DASHBOARD] = this.views.panelDashboard;
    this.viewsMap[this.PANELS.PROTOCOL_LIST] = this.views.panelProtocolList;
    this.viewsMap[this.PANELS.PROTOCOL_DETAIL] = this.views.panelProtocolDetail;
    this.viewsMap[this.PANELS.EXPERIMENT_LIST] = this.views.panelExperimentList;
    this.viewsMap[this.PANELS.PROTOCOL_EDITOR] = this.views.panelProtocolEditor;
    this.viewsMap[this.PANELS.EXPERIMENT_EDITOR] = this.views.panelExperimentEditor;
    this.viewsMap[this.PANELS.EXPERIMENT_MONITOR] = this.views.panelExperimentMonitor;
    this.viewsMap[this.PANELS.OPTICS_MONITOR] = this.views.panelOpticsMonitor;
    this.viewsMap[this.PANELS.PLATE_CONTROL_MONITOR] = this.views.panelPlateControlMonitor;
    
    this.viewsMap[this.PANELS.TEMPLATE] = this.views.panelTemplate;
    this.views.panelDashboard.onAppear();
  }
  
  /* Public methods */
  
  // Returns current panel component
  pushPanel (panelId, message) {
    this._confirmLeavePanel(()=>{
      const toPanel = this.getPanel(panelId);
      if (toPanel && toPanel.onAppear) {
        toPanel.onAppear(message);
      } else {
        console.log("onAppear not defined.");
      }
      this.panelStack.push(panelId);
      if (this.panelContainer) {
        this.panelContainer.presentPanel(panelId, toPanel);
        this._didNavigate();
      } else {
        console.log("PushPanel panelContainer is null.");
      }
    });
  }
  backPanel (message) {
    if (this.panelStack.length < 2) {
      return;
    }
    this._confirmLeavePanel(()=>{
      this.panelStack.pop();
      if (this.panelContainer) {
        const panelId = this.panelStack[this.panelStack.length-1];
        const toPanel = this.getPanel(panelId);
        if (toPanel && toPanel.onAppear) {
          toPanel.onAppear(message);
        } else {
          console.log("onAppear not defined.");
        }
        this.panelContainer.presentPanel(panelId, toPanel);
        this._didNavigate();
      }
    });
  }
  home (message) {
    if (this.panelStack.length < 2) {
      return;
    }
    this._confirmLeavePanel(()=>{
      while (this.panelStack.length > 1) {
        this.panelStack.pop();
      }
      if (this.panelContainer) {
        const panelId = this.panelStack[this.panelStack.length-1];
        const toPanel = this.getPanel(panelId);
        if (toPanel && toPanel.onAppear) {
          toPanel.onAppear(message);
        } else {
          console.log("onAppear not defined.");
        }
        this.panelContainer.presentPanel(panelId, toPanel);
        this._didNavigate();
      }
    });
  }
  // Call custom confirmation handler and wait for its result.
  _confirmLeavePanel (callback) {
    const fromPanel = this._currentPanel();
    if (fromPanel.confirmLeave) {
      fromPanel.confirmLeave(callback);
      if (fromPanel.onDisappear) {
        fromPanel.onDisappear();
      }
    } else {
      callback();
      if (fromPanel.onDisappear) {
        fromPanel.onDisappear();
      }
    }
  }
  _currentPanel () {
    return this.getPanel(this.panelStack[this.panelStack.length-1]);
  }
  _didNavigate () {
    if (this.navigationHandler) {
      this.navigationHandler(this.panelStack);
    }
  }
  
  newProtocol () {
    console.log("AppState.newProtocol");
  }
  
  
  /* Event handler registration */
  setPanelContainer (container) {
    this.panelContainer = container;
  }
  
  toast (context, title, message) {
    context.$bvToast.toast(message, {
      title: title
    })
  }

  loadProtocols (callback, onError) {
    Util.requestData(
      "https://api-ninja.hisa.dev/protocol?uid=uk&pretty=true",
      // "data/protocols.json",
       null, "GET", (data)=>{
      callback(data)
    }, onError);
  }
  protocolTemplate () {
    return PROTOCOL_TEMPLATE;
  }
  
  /* TODO Separate API call and UI control */
  
  init () {
    /* HTTP request methods */
    device.init();
  }
  
  
}
const appState = new AppState();
module.exports = appState;