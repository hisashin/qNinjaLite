<template>
  <div>
    <header class="header">
      <div class="header__content">
        <h1 class="header__title">
          qNinja LITE
        </h1>
        <!-- Device monitor -->
          <div class="header__device">
          <span class="header__device">
            <span v-if="connectionStatus.device.connected">🟢 Device is online</span>
            <span v-if="!connectionStatus.device.connected">⚪Device is offline</span>
          </span>
          <span class="header__user">
            <a href="javascript:void(0)" v-if="!connectionStatus.server.connected" @click="openLogin()">Logout</a>
            <span v-if="connectionStatus.server.connected">
              👤{{ username }}
              <a href="javascript:void(0)" @click="logout()">Logout</a>
            </span>
            
          </span>
          <!-- Network -->
        </div>
      </div>
    </header>
    <main class="main">
      <!-- To dialog -->
      
      <b-modal
        id="login-modal"
        title="Login"
        hide-footer
      >
        <div>
        username <input
          v-model="username"
          type="text"
        />
        </div>
        <div>
        password <input
          v-model="password"
          type="text"
        />
        </div>
        <div>
        Thing ID <input
          v-model="thingId"
          type="text"
        />
        </div>
        <div>
        <label>
        <input type="checkbox" v-model="autologin" />
        Automatic login
        </label>
        </div>
        <div>
          <b-button variant="primary" class="ml-1" @click="login()">
            Login
          </b-button>
        </div>
      </b-modal>
      <!--<TheDeviceSummary v-show="selectedPanel!=panels.EXPERIMENT_MONITOR"/>-->
      <nav 
        v-if="panelTitle && selectedPanel!=panels.DASHBOARD"
        class="panel-nav">
        <BackButton v-show="backEnabled" />
        <h2 class="panel-nav__title">
          {{ panelTitle }}
        </h2>
      </nav>
      <!-- Panels -->
      <TheDashboard ref="panelDashboard" v-show="selectedPanel==panels.DASHBOARD" />
      <TheExperimentMonitor ref="panelExperimentMonitor" v-show="selectedPanel==panels.EXPERIMENT_MONITOR" />
      <TheProtocolEditor ref="panelProtocolEditor" v-show="selectedPanel==panels.PROTOCOL_EDITOR" />
    </main>
    <footer class="footer">
    </footer>
  </div>
</template>
<script>

import TheDashboard from './panels/TheDashboard.vue'
import TheExperimentMonitor from './panels/TheExperimentMonitor.vue'
import TheProtocolEditor from './panels/TheProtocolEditor.vue'
import BackButton from './BackButton.vue';
import device from "../lib/Device.js";
import appState from "../lib/AppState.js";
import login from "../lib/Login.js";

export default {
  name: 'TheMain',
  components: {
    TheDashboard,
    TheExperimentMonitor,
    TheProtocolEditor,
    BackButton
  },
  data() {
    return {
      isIdle:true,
      panels:appState.PANELS,
      selectedPanel:appState.PANELS.DASHBOARD,
      backEnabled: false,
      panelTitle: "",
      connectionStatus: device.Connection.DISCONNECTED,
      wellTemp: "-",
      initialState:null,
      username:"",
      password:"",
      thingId:"",
      autologin:false,
      autologinDisabled:false // For test
    }
  },
  created: function () {
    appState.setPanelContainer(this);
    device.network.connectionStatus.observe((connStatus)=>{
      this.connectionStatus = connStatus;
    });
    device.deviceState.observe((data)=>{
      if (!data) return;
      if (!this.initialState) {
        this.initialState = data
        console.log("Initialstate=%s", data.label)
        if (data.has_experiment && window.confirm("The device has an ongoing experiment. Do you like to open the monitor?")) {
          appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR, {continue:true});
        }
      }
    });

    device.subscribe(device.device_data_topic_filter("state"), (topic, data)=>{
        if (!this.initialState) {
          this.initialState = data
          console.log("Initialstate=%s", data.label)
          if (data.has_experiment && window.confirm("The device has an ongoing experiment. Do you like to open the monitor?")) {
            appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR, {continue:true});
          }
        }
    });
    device.subscribe(device.experiment_data_topic_filter("event"), (topic, data)=>{
      if (data.label == "start") {
        appState.toast(this, "qNinja LITE", "The experiment has started.");
      }
      if (data.label == "resume") {
        appState.toast(this, "qNinja LITE", "The experiment has been resumed.");
      }
      if (data.label == "cancel") {
        appState.toast(this, "qNinja LITE", "The experiment was cancelled.");
      }
      if (data.label == "finish") {
        // Nothing to do 
      }
      if (data.label == "pause") {
        appState.toast(this, "qNinja LITE", "The experiment has been paused.");
      }
      if (data.label == "complete") {
        // Nothing to do
      }
    });
    appState.setNavigationHandler((panelStack)=>{
      this.backEnabled = (panelStack.length > 1);
    
    });
    // Connection check
    const loginInfo = login.getLoginInfo();
    this.username = loginInfo.username;
    this.password = loginInfo.password;
    this.password = loginInfo.password;
    this.thingId = loginInfo.thingId;
    if (login.isFilled() && loginInfo.autologin && !this.autologinDisabled) {
      device.connect(loginInfo);
    } else {
      this.$nextTick(()=>{
        this.$bvModal.show('login-modal');
      });
    }
  },
  mounted: function () {
    appState.setViews(this.$refs);
  
  },
  methods: {
    presentPanel(panel, toPanel) {
      this.selectedPanel = panel;
      this.panelTitle = (toPanel.title) ? toPanel.title() : "N/A";
    },
    showTemplate () {
      appState.pushPanel(appState.PANELS.TEMPLATE);
      
    },
    home () {
      appState.home();
    },
    revealDetailLatestExperiment () {
      appState.revealDetailLatestExperiment(this);
    },
    openLogin () {
        this.$bvModal.show('login-modal');

    },
    login () {
      console.log("login")
      const loginInfo = {
        username: this.username,
        password: this.password,
        thingId: this.thingId,
        autologin: this.autologin,
      };
      login.setLoginInfo(loginInfo);
      if (login.isFilled()) {
        device.connect(loginInfo);
        this.$bvModal.hide('login-modal');
      }
    },
    logout () {
      login.logout();
      location.reload();
    }
  }
}
</script>