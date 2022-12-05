<template>
  <div class="panel panel--dashboard">
    <div class="panel__menu">
    </div>
    <ProgressMonitor ref="progressMonitor" @home="home"/>
    <b-button class="mr-1 btn-sm"
      @click.stop="back">
      Back
    </b-button>
    <section class="section" v-show="protocol">
      <div class="section__body">
        <div class="item item--tabbed">
          <b-tabs pills content-class="item--tabbed__content" nav-wrapper-class="item--tabbed__tabs" active>
            <b-tab title="Temperature">
              <div style="width:800px;height:640px;">
                <TemperatureChart ref="temperatureChart" />
              </div>
            </b-tab>
            <b-tab title="Protocol">
              <div style="width:800px;height:640px;">
                <ProtocolDetail ref="protocolDetail" :protocol="protocol" />
              </div>
            </b-tab>
            <b-tab title="AmplificationChart">
              <div style="width:800px;height:640px;">
                <AmplificationChart ref="amplificationChart" />
              </div>
            </b-tab>
          </b-tabs>
        </div>
      </div>
    </section>
  </div>
</template>
<script>

import appState from "../../lib/AppState.js";
import device from "../../lib/Device.js";
import Util from "../../lib/Util.js";
import hardwareConfig from "../../lib/HardwareConfig.js";
import TemperatureChart from '../ExperimentMonitor/TemperatureChart.vue'
import AmplificationChart from '../ExperimentMonitor/AmplificationChart.vue'
import ProgressMonitor from '../ExperimentMonitor/ProgressMonitor.vue'
import ProtocolDetail from '../ProtocolDetail.vue'
export default {
  name: 'TheExperimentMonitor',
  components: {
    TemperatureChart,
    ProgressMonitor,
    AmplificationChart,
    ProtocolDetail
  },
  props: {
  },
  data() {
    return {
      subs: [],
      protocol: null,
      ampChartReady: false
    }
  },
  mounted: function () {
  },
  created: function () {
    device.protocol.observe((protocol)=>{
      this.protocol = protocol
    });
  },
  methods: {
    subscribe (topic, handler) {
      this.subs.push(device.subscribe(topic, handler));
    },
    onAppear (message) {
      if (!this.ampChartReady) {
        this.$refs.amplificationChart.setHardwareConf(hardwareConfig);
        this.$refs.amplificationChart.setAppearanceConf(this.getAppearanceConf());
        this.ampChartReady = true;
      }
      this.$refs.amplificationChart.clear();
      this.$refs.temperatureChart.clear();
      this.subscribe(device.experiment_data_topic_filter("fluo"), (topic, obj)=>{
        this.$refs.amplificationChart.add(obj);
      });
      if (message && message.continue) {
        console.log("Continue", message.continue)
        device.publish(device.device_command_topic("req-experiment"), {}, (res)=>{
          console.log("Received req-experiment response.");
          console.log(res);
          this.protocol = res.g.p;
        });
      }
    },
    home () {
      console.log("Emitted home event");
      console.log("Unsub all")
      this.subs.forEach((subId)=>{device.unsubscribe(subId)});
      appState.home();
    },
    back () {
      console.log("Unsub all")
      this.subs.forEach((subId)=>{device.unsubscribe(subId)});
      appState.backPanel();

    },
    getAppearanceConf () {
      console.log("TheExperimentMonitor.getAppearanceConf")
      let appearances = [];
      this.well_appearance = [];
      console.log(hardwareConfig);
      for (let w=0; w<hardwareConfig.wells.count; w++) {
        let obj = {
          v: true,
          c: Util.defaultPalette[w]
        };
        this.well_appearance.push(obj);
      }
      for (let ch=0; ch < hardwareConfig.channels.count; ch++) {
        appearances.push(this.well_appearance);
      }
      console.log(appearances)
      return appearances;

    },
    title () { return "Experiment Monitor" }
  }
}
</script>