<template>
  <div class="panel panel--dashboard">
    <div class="panel__menu">
    </div>
    <section class="section section--dashboard-protocols">
      <div>
        <select v-model="protocolIndex" :disabled="!protocols || protocols.length==0">
            <option v-for="(item, index) in protocols" :key="index" :value="index">
              {{ item.b }}
            </option>
        </select>
        <b-button variant="primary" class="ml-1 btn-sm" :disabled="!protocols || protocols.length==0"
          @click.stop="selectProtocol">
          Use this protocol
        </b-button>
      </div>
      <div>
        <b-button variant="primary" class="ml-1 btn-sm"
          @click.stop="newProtocol">
          + New protocol
        </b-button>

      </div>

      <div>
        <b-button v-b-toggle.advanced_settings variant="link">Advanced settings</b-button>
        <b-collapse id="advanced_settings" class="mt-2">
          <h2>PID Tuning</h2>
          <textarea v-model="pid_config" cols="60" rows="4"></textarea>
          <br/>
          <!--
            :disabled="!connectionStatus.device.connected"
            -->
          <b-button class="mr-1 btn-sm"
            @click.stop="confPID">
            Set PID Constants
          </b-button>
          <b-button class="mr-1 btn-sm"
            @click.stop="validatePID">
            Validate
          </b-button>
          <h2>Calibration (ToDo)</h2>
        </b-collapse>
      </div>

    </section>
  </div>
</template>
<script>

import appState from "../../lib/AppState.js";
import device from "../../lib/Device.js";
import pidValidator from "../../lib/pid_validation.js";
/*
const DEFAULT_PID_CONF = {
  "consts":[
    {"kp":0.3, "ki":0, "kd":0, "max_value":55},
    {"kp":0.3, "ki":0, "kd":0, "min_value":55, "max_value":80},
    {"kp":0.3, "ki":0, "kd":0, "min_value":80}
  ]
}
  */

const DEFAULT_PID_CONF = {
  "consts":[{"kp":0.12,"ki":0.009,"kd":0.02}]
}
  
const DEFAULT_PROTOCOL = 
{
  name:"Default Protocol",
  "steps":[
    { "t":72, "d":120.0, "c":1, "i":5 },
    { "t":84, "d":30.0, "c":1, "i":5 }
  ],
  "h":20
};
export default {
  name: 'TheDashboard',
  components: {
  },
  props: {
  },
  data() {
    return {
      connectionStatus: device.Connection.INITIAL,
      pid_config:JSON.stringify(DEFAULT_PID_CONF),
      deviceState:{},
      protocols:[],
      protocolIndex:0
    }
  },
  created: function () {
    device.network.connectionStatus.observe((status)=>{
      this.connectionStatus = status;
    });
    device.deviceState.observe((state)=>{
      this.deviceState = state;
      appState.loadProtocols((data)=>{
        console.log("LOAD_PROTOCOLS")
        let protocols = data.data.Items;
        console.log(data)
        this.protocols = protocols;
      }, ()=>{
        // onError
        appState.toast(this, "Error", "Failed to load protocols.");
      });
    });
  },
  methods: {
    onAppear () {
      console.log("TheDashboard.onAppear")
    },
    selectProtocol () {
      appState.pushPanel(appState.PANELS.PROTOCOL_EDITOR, {protocol:this.protocols[this.protocolIndex]});
    },
    newProtocol () {
      appState.pushPanel(appState.PANELS.PROTOCOL_EDITOR, {protocol:appState.protocolTemplate()});
    },
    validatePID(){
      const errors = pidValidator.validate(JSON.parse(this.pid_config))
      console.log(errors)
      if (errors.length > 0) {
        appState.toast(this, "PID Config Error", JSON.stringify(errors));
      } else {
        appState.toast(this, "PID Config", "OK");
      }

    },
    confPID () {
      console.log(this.pid_config);
      const pidObj = JSON.parse(this.pid_config);
      const errors = pidValidator.validate(JSON.parse(this.pid_config))
      if (errors.length > 0) {
        console.log("Error.")
        appState.toast(this, "PID Config Error", JSON.stringify(errors));
        return;
      }
      device.confPID(pidObj, ()=>{
        appState.toast(this, "PID Config", "PID constants have changed.");
      })
    },
    title () { return "Dashboard" }
  }
}
</script>