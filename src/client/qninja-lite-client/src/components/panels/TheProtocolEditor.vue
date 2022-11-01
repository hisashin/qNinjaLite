<template>
  <div class="panel">
    <div class="panel__menu"></div>
    <section class="section" v-show="protocol">
      <div class="section__body">
        <div class="item">
          <div>
            Name:
            <input
              v-model="protocol.name"
              type="text"
              v-on:input="onChangeProtocol()"
            />
          </div>
          <div>
            <input
              v-model.number="protocol.final_hold_temp"
              v-on:input="onChangeProtocol()"
              class="input-temp input-temp--2"
            />℃
          </div>
          <b-button variant="secondary" class="ml-1" @click="addStepFirst()">
            Add
          </b-button>
          <ul>
            <template v-for="(step, index) in protocol.steps">
              <li :key="index">
                <div>
                  Step {{ index + 1 }} Temp:<input
                    v-model.number="step.temp"
                    v-on:input="onChangeProtocol()"
                    class="input-temp"
                    type="number"
                  />℃ Duration:<input
                    v-model.number="step.duration"
                    v-on:input="onChangeProtocol()"
                    class="input-temp"
                    type="number"
                  />s 
                  <label>Data collection:
                  <input
                    type="checkbox"
                    v-bind:id="'data_collection' + index"
                    v-model="step.data_collection"
                    v-bind:key="'data_collection' + index"
                    :true-value="trueValue"
                    :false-value="falseValue"
                  /></label>
                  Data collection interval:<input
                    v-model.number="step.data_collection_interval"
                    v-on:input="onChangeProtocol()"
                    class="input-temp"
                    type="number"
                  />s
                </div>
                <div>
                  <b-button
                    variant="secondary"
                    class="ml-1"
                    @click="addStep(index)"
                  >
                    Add
                  </b-button>
                  <b-button
                    variant="secondary"
                    class="ml-1"
                    @click="deleteStep(index)"
                  >
                    Delete
                  </b-button>
                </div>
              </li>
            </template>
          </ul>
          <b-button variant="primary" class="ml-1" @click="save()">
            Save
          </b-button>
          <b-button :disabled="!(deviceState && deviceState.start_available)" variant="primary" class="ml-1" @click="saveAndRun()">
            Save and Run
          </b-button>
        </div>
      </div>
    </section>
  </div>
</template>
<script>
import appState from "../../lib/AppState.js";
import device from "../../lib/Device.js";

const DEFAULT_STEP = {
  temp: 42,
  duration: 42.0,
  data_collection: 1,
  data_collection_interval: 5,
};
export default {
  name: "TheExperimentMonitor",
  components: {},
  props: {},
  data() {
    return {
      protocol: appState.protocolTemplate(),
      trueValue: 1,
      falseValue: 0,
      deviceState:{},
    };
  },
  mounted: function () {},
  created: function () {
    console.log("TheExperimentMonitor.created");
    device.deviceState.observe((state)=>{
      this.deviceState = state;
    });
  },
  methods: {
    subscribe(topic, handler) {
      this.subs.push(device.subscribe(topic, handler));
    },
    onAppear(message) {
      console.log(message)
      if (message && message.protocol) {
        console.log("Edit protocol")
        console.log(message.protocol)
        this.protocol = message.protocol;
      }
    },
    back() {
      appState.backPanel();
    },
    save: function () {
      console.log(JSON.parse(JSON.stringify(this.protocol)));
    },
    saveAndRun: function () {
      console.log("TheProtocolEditor.saveAndRun");
      device.start(this.protocol, ()=>{
        console.log("TheProtocolEditor.saveAndRun cb");
        device.protocol.set(this.protocol);
        appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR);
      });
    },
    onChangeProtocol: function () {
      console.log("onChangeProtocol");
      console.log(JSON.parse(JSON.stringify(this.protocol)));
    },
    addStepFirst: function () {
      console.log("TODO addStepFirst");
      this.addStep(-1);
    },
    addStep: function (after) {
      this.protocol.steps.splice(after + 1, 0, DEFAULT_STEP);
      this.onChangeStructure();
      console.log("TODO addStep(%d)", after + 1);
    },
    deleteStep: function (index) {
      if (window.confirm("Are you sure you want to delete this step?")) {
        this.protocol.steps.splice(index, 1);
        this.onChangeStructure();
      }
    },
    onChangeStructure() {
      this.protocol = JSON.parse(JSON.stringify(this.protocol));
    },
    title() {
      return "Protocol Editor";
    },
  },
};
</script>