<template>
  <div class="panel panel--dashboard">
    <div class="panel__menu">
    </div>
    <section class="section section--dashboard-protocols">
      <h2>qNinjaLITE</h2>
      <div>
        <b-button class="mr-1 btn-sm"
          @click.stop="editProtocol">
          Edit Protocol
        </b-button>
        <b-button class="mr-1 btn-sm"
          @click.stop="startExperiment">
          Run Default Experiment
        </b-button>
      </div>
      <div>
        <textarea v-model="pid_config" cols="80" rows="6"></textarea>
        <br/>
        <b-button class="mr-1 btn-sm"
          @click.stop="confPID">
          Set PID Constants
        </b-button>
      </div>
    </section>
  </div>
</template>
<script>

import appState from "../../lib/AppState.js";
import device from "../../lib/Device.js";
/*
const DEFAULT_PID_CONF = [
    {"kp":0.3, "ki":0, "kd":0, "max_value":55},
    {"kp":0.3, "ki":0, "kd":0, "min_value":55, "max_value":80},
    {"kp":0.3, "ki":0, "kd":0, "min_value":80}
  ]
  */

const DEFAULT_PID_CONF = [
    {"kp":0.3, "ki":0, "kd":0}
  ]
  
const DEFAULT_PROTOCOL = {name:"Default Protocol","steps":[
  {"temp":72,"duration":120.0,"data_collection":1,"data_collection_interval":5},
  {"temp":84,"duration":30.0,"data_collection":1,"data_collection_interval":5}
  ],"final_hold_temp":20};
// const DEFAULT_PROTOCOL = {name:"Default Protocol","steps":[{"temp":50,"duration":10.0,"data_collection":1,"data_collection_interval":5}],"final_hold_temp":20};
export default {
  name: 'TheDashboard',
  components: {
  },
  props: {
  },
  data() {
    return {
      pid_config:JSON.stringify(DEFAULT_PID_CONF)
    }
  },
  created: function () {
  },
  methods: {
    onAppear () {
      console.log("TheDashboard.onAppear")
    },
    startExperiment () {
      console.log("TheDashboard.startExperiment")
      const protocol = DEFAULT_PROTOCOL;
      device.start(protocol, ()=>{
        console.log("TheProtocolEditor.saveAndRun cb");
        device.protocol.set(protocol);
        appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR);
      });
    },
    editProtocol () {
      console.log("TheDashboard.editProtocol")
      appState.pushPanel(appState.PANELS.PROTOCOL_EDITOR);
    },
    confPID () {
      console.log(this.pid_config);
      device.confPID(JSON.parse(this.pid_config), ()=>{
        alert("PID config changed.")
      })
    },
    title () { return "Dashboard" }
  }
}
</script>