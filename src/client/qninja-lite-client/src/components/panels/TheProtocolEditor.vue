<template>
  <div class="panel">
    <div class="panel__menu"></div>
    <section class="section" v-show="protocol">
      <div class="section__body">
        <div class="item">
          <div>
            <span :class="'validation-label validation__name'"/>
            Name
            <input
              v-model="item.b"
              v-show="isNew"
              type="text"
              v-on:input="onChangeProtocol()"
            />
            <span v-show="!isNew">{{ item.b }}</span>
          </div>
          <div>
            <span :class="'validation-label validation__final_hold_temp'"/>
            Final hold temp <input
              v-model.number="protocol.h"
              v-on:input="onChangeProtocol()"
              class="input-temp input-temp--2"
            />℃
          </div>
          <b-button pill size="sm" variant="secondary" class="ml-1" @click="addStepFirst()">
            + Step
          </b-button>
          <span :class="'validation-label validation__steps'"/>
          <ul>
            <template v-for="(step, index) in protocol.s">
              <li :key="index">
                <div>
                  <div>
                    <span :class="'validation-label validation__steps__' + index" />
                    <strong>Step {{ index + 1 }} </strong>
                    <b-button
                      variant="link"
                      class="ml-1" size="sm"
                      @click="deleteStep(index)"
                    >
                      Delete
                    </b-button>
                  </div>
                  Temp
                  <span :class="'validation-label validation__steps__' + index + '__temp'" />
                  <input
                    v-model.number="step.t"
                    v-on:input="onChangeProtocol()"
                    class="input-temp--2"
                    type="number"
                    :min="minTemp" :max="maxTemp"
                  />℃ Duration
                  <span :class="'validation-label validation__steps__' + index + '__duration'" />
                  <input
                    v-model.number="step.d"
                    v-on:input="onChangeProtocol()"
                    class="input-temp"
                    type="number"
                    min="0"
                  />s 
                  <label>
                  <input
                    type="checkbox"
                    v-bind:id="'c' + index"
                    v-model="step.c"
                    v-bind:key="'c' + index"
                    :true-value="trueValue"
                    :false-value="falseValue"
                  />Data collection</label>
                  (Interval
                  <span :class="'validation-label validation__steps__' + index + '__data_collection_interval'" /><input
                    v-model.number="step.i"
                    v-on:input="onChangeProtocol()"
                    class="input-temp"
                    type="number"
                    min="5"
                  />s)
                </div>
                <div>
                  <b-button pill size="sm"
                    variant="secondary"
                    class="ml-1"
                    @click="addStep(index)"
                  >
                    + Step
                  </b-button>
                </div>
              </li>
            </template>
          </ul>
          <b-button :disabled="hasError || !(deviceState && deviceState.i)" variant="primary" class="ml-1" @click="saveAndRun()">
            Run
          </b-button>
          <b-button variant="secondary" class="ml-1" @click="saveProtocol()">
            Save
          </b-button>
          <b-button variant="secondary" class="ml-1" @click="deleteProtocol()" v-show="!isNew">
            Delete
          </b-button>
          <b-button variant="secondary" class="ml-1" @click="debug()" v-show="!isNew">
            Debug (eid)
          </b-button>
        </div>
      </div>
    </section>
  </div>
</template>
<script>
import appState from "../../lib/AppState.js";
import device from "../../lib/Device.js";
import protocolValidator from "../../lib/protocol_validation.js";

const DEFAULT_STEP = {
  t: 50.0,
  d: 180.0,
  c: 1, // Data collection flag
  i: 10, // Data collection interval
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
      minTemp: 4,
      maxTemp:98,
      deviceState:{},
      hasError: false,
      item: {},
      isNew: false,
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
      if (message && message.protocol /* Protocol item */) {
        console.log("Edit protocol item=")
        console.log(JSON.stringify(message.protocol))
        console.log("Edit protocol message.protocol=")
        console.log(JSON.stringify(message.protocol.p))
        this.item = message.protocol;
        this.item.tid = device.getThingID();
        this.isNew = false;
        this.protocol = message.protocol.p;
      } else {
        // New item
        this.item = {"tid":device.getThingID()};
        this.item.p = appState.protocolTemplate();
        this.isNew = true;
      }
    },
    back() {
      appState.backPanel();
    },
    saveProtocol: function () {
      const errors = protocolValidator.validate(this.protocol)
      this._processValidationResult(errors);
      if (errors.length == 0) {
        console.log(JSON.stringify(this.protocol));
        console.log(JSON.stringify(this.item));
        if (!this.item) {
          this.item = {};
        }
        device.saveProtocol(this.item, ()=>{
          appState.toast(this, "Saved", "Protocol saved.");
        }, ()=>{
          appState.toast(this, "Error", " Failed to save the protocol.");
        });
      }
    },
    saveAndRun: function () {
      device.start(this.protocol, ()=>{
        device.protocol.set(this.protocol);
        appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR);
      });
    },
    deleteProtocol: function () {
      if (window.confirm("Are you sure you want to delete this protocol?")) {
        device.deleteProtocol(this.item, ()=>{
          appState.toast(this, "Deleted", "Protocol deleted.");
          appState.backPanel();
        }, ()=>{
          appState.toast(this, "Error", " Failed to delete the protocol.");
        });
      }
    },
    validate: function () {
      console.log("TheProtocolEditor.validate");
      const errors = protocolValidator.validate(this.protocol)
      this._processValidationResult(errors);
    },
    _setMessage: function (selector, message) {
      let elements = document.querySelectorAll(selector);
      for (let i=0; i<elements.length; i++) {
        elements[i].innerHTML = message;
      }
      if (elements.length == 0) {
        console.warn("No element for selector %s", selector);
      }
    },
    _pathToClassName: function(path) {
      return ".validation__" + path.join("__");
    },
    _processValidationResult: function (result) {
      this._setMessage(".validation-label", "");
      console.log(result)
      for (let item of result) {
        console.log("Path=" + this._pathToClassName(item.path));
        this._setMessage(this._pathToClassName(item.path), '<div class="validation-label__content">'+item.message+'</div>')
      }
      // this._setMessage(".validation-label", '<div class="validation-label__content">DEBUG</div>'); // For style debug
      this.hasError = (result.length > 0);
    },
    /* Editing */
    onChangeProtocol: function () {
      this.validate();
    },
    addStepFirst: function () {
      this.addStep(-1);
    },
    addStep: function (after) {
      this.protocol.s.splice(after + 1, 0, DEFAULT_STEP);
      this._onChangeStructure();
    },
    deleteStep: function (index) {
      if (window.confirm("Are you sure you want to delete this step?")) {
        this.protocol.s.splice(index, 1);
        this._onChangeStructure();
      }
    },
    _onChangeStructure: function() {
      this.protocol = JSON.parse(JSON.stringify(this.protocol));
    },
    title() {
      return "Protocol Editor";
    },
  },
};
</script>