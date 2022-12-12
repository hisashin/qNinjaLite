<template>
  <div style="width:100%">
    <div v-if="protocol==null">Protocol is null</div>
    <div v-if="progress==null">Progress is null</div>
    <div v-if="deviceState==null">DeviceState is null</div>
    <div class="progress-monitor"
      v-if="protocol!=null && progress!=null && deviceState != null">
      <!-- Modal dialog to finish the experiment -->
      <b-modal
        id="finish-modal"
        title="The experiment is complete"
        hide-footer
      >
        <b-button 
          variant="primary"
          block @click="finish"
        >
          Stop the heater
        </b-button>
      </b-modal>
      <!-- Modal dialog to show the experiment result -->
      <b-modal
        id="result-modal"
        title="The experiment is complete!"
        hide-footer
      >
        <b-button 
          variant="primary"
          block @click="home"
        >
          Home
        </b-button>
        <b-button 
          variant="primary"
          block @click="result"
        >
          Result
        </b-button>
      </b-modal>
      <div class="progress-monitor__row">
        <ul class="progress-meter">
          <!-- List of stages -->
          <li 
            class="progress-meter__item"
            v-for="(step, index) in protocol.s" :key="index" 
            v-bind:class="{ 'progress-meter__item--notyet': progress.s<index,'progress-meter__item--current': progress.s==index,'progress-meter__item--done': progress.s>index }">
            <span class="progress-meter__item progress-meter__label">{{ step.t }}℃ 
              <span
                v-if="progress.s==index && progress.label=='hold'">{{ stepElapsedSec }}</span>/{{ step.d }}s
            </span>
            <div
              v-if="progress.s==index" class="progress-meter__item progress-meter__detail">{{ progress.label }}</div>
          </li>
          <li
            class="progress-meter__item"
            v-bind:class="{ 'progress-meter__item--current': progress.label=='final_hold', 'progress-meter__item--notyet': progress.label!='final_hold' }">
            <span class="progress-meter__item progress-meter__label">Final hold</span>
          </li>
        </ul>
      </div>
      
      <div class="progress-monitor__row">
        <div class="temperature-monitor">
          <div class="temperature-monitor__label">
            Plate
            <div class="temperature-monitor__value">{{ plateTemp }} 
            <template 
              v-if="step!=null && step.t!=null">/{{step.t}}℃</template>
            </div>
          </div>
          <div class="temperature-monitor__meter">
            <meter class="temperature-monitor__meter-body"
              min="20"
              max="100"
              :value="progress.p"></meter>
            <div class="temperature-monitor__meter-scale">
              <div class="temperature-monitor__meter-scale-element">20</div>
              <div class="temperature-monitor__meter-scale-spacer"></div>
              <div class="temperature-monitor__meter-scale-element">40</div>
              <div class="temperature-monitor__meter-scale-spacer"></div>
              <div class="temperature-monitor__meter-scale-element">60</div>
              <div class="temperature-monitor__meter-scale-spacer"></div>
              <div class="temperature-monitor__meter-scale-element">80</div>
              <div class="temperature-monitor__meter-scale-spacer"></div>
              <div class="temperature-monitor__meter-scale-element">100</div>
            </div>
          </div>
        </div>
      </div>
      <div>
        Air {{ airTemp }}C
        
      </div>
      <div class="progress-monitor__row" v-if="progress.label!='final_hold'">
        <div class="time-monitor">
          <div class="time-monitor__elapsed">{{ elapsedTimeLabel }} </div>
          <div class="time-monitor__estimated"> / {{ totalTimeLabel }}</div>
          <div class="time-monitor__remaining"> ({{ remainingTimeLabel }} left)</div>
          <meter class="time-monitor__meter" 
            min="0"
            :max="remainingTime+elapsedTime"
            :value="progress.e"></meter>
          
          <template v-if="deviceState != null">
          <button class="time-monitor__button" v-if="deviceState.u" @click="pause">Pause</button>
          <button class="time-monitor__button" v-if="deviceState.r" @click="resume">Resume</button>
          <button class="time-monitor__button" v-if="deviceState.c" @click="cancel">Cancel</button>
          <button class="time-monitor__button" v-if="deviceState.f" @click="finish">Finish</button>
          <button class="time-monitor__button" @click="dl('json')">JSON</button>
          <button class="time-monitor__button" @click="dl('csv')">CSV</button>
          </template>
          progressCount={{progressCount}},
          progressFreqCount={{progressFreqCount}}
          
        </div>
      </div>
    </div>
  </div>
</template>


<script>
import device from "../../lib/Device.js";
import appState from "../../lib/AppState.js";
import Util from "../../lib/Util.js";
import Constants from "../../lib/constants.js";
var numeral = require('numeral');

let LABEL_MAP = {};
LABEL_MAP[Constants.StageType.HOLD] = "Hold";
LABEL_MAP[Constants.StageType.QPCR] = "qPCR";
LABEL_MAP[Constants.StageType.MELT_CURVE] = "Melt Curve";
LABEL_MAP[Constants.StageType.PCR] = "Normal PCR";

export default {
  name: 'ProgressMonitor',
  data() {
    return {
      protocol:null,
      deviceState: null,
      progress: null,
      stage: { step:[]},
      step: {},
      stepElapsedSec: "",

      elapsedTime: 0,
      remainingTime: 0,
      totalTime: 0,
      elapsedTimeLabel: "-",
      remainingTimeLabel: "-",
      totalTimeLabel: "-",

      plateTemp: "-",
      airTemp: "-",

      progressCount:0,
      progressFreqCount:0
    }
  },
  computed: {
  },
  created: function () {
    device.subscribe(device.experiment_data_topic_filter("progress"), (topic, data, id)=>{
      this.applyProgress(data);
      this.progressCount ++;

    });
    device.subscribe(device.experiment_data_topic_filter("progress-freq"), (topic, data, id)=>{
      this.applyProgress(data);
      this.progressFreqCount ++;
    });
    device.protocol.observe((protocol)=>{
      console.log("ProgressMonitor.protocol set.");
      console.log(protocol)
      this.protocol = protocol;
    });
    device.deviceState.observe((state)=>{
      console.log("STATE " + JSON.stringify(state))
      this.closeModals(); 
      if (state && state.b == "online") {
        console.log("STATE ???")
      }
      else if (this.deviceState && !this.deviceState.b!="complete" && state.b=="complete") {
        this.$bvModal.show('finish-modal');
      }
      if (this.deviceState && this.deviceState.x && !state.x) {
        // The device become "idle"
        this.$bvModal.show('result-modal');
      }
      this.deviceState = state;
    });
  },
  methods: {
    stageLabel (stageType) {
      return LABEL_MAP[stageType];
    },
    reset () {
      this.closeModals(); 
    },
    getEstimatedTransitionTimeMs (fromTemp, toTemp) {
      return Math.abs(fromTemp - toTemp) * 1000
    },
    getRemainingTimeMs (protocol, progress) {
      let time = 0;
      if (!protocol) {
        throw "Protocol is null."
      }
      if (!protocol.s) {
        throw "Protocol has no steps."
      }
      if (!progress) {
        throw "Progress is null."
      }
      if (progress.l == device.StepLabels.FINAL_HOLD) {
        // Experiment is complete
        return 0;
      }
      protocol.s.forEach((step, index)=>{
        if (progress.s  == index) {
          // Current step
          if (progress.l == device.StepLabels.RAMP) {
            // Ramp time
            time += this.getEstimatedTransitionTimeMs(progress.p, step.t);
            // Hold time
            time += step.d * 1000;
          } else if (progress.l == device.StepLabels.HOLD) {
            time += (step.d * 1000 - progress.d)
          } else {
            throw "Unknown progress.l value: " + progress.l;

          }
        } else if (progress.s  < index) {
          // Ramp time
          time += this.getEstimatedTransitionTimeMs(protocol.s[index-1].t, step.t);
          // Hold time
          time += step.d * 1000;
        }
      });
      if (isNaN(time)) {
        console.log("NaN???")
      }
      return time;

    },
    applyProgress (progress) {
      // TODO string labels are deprecated.
      if (progress.l == "ramp") { progress.l = 1 }
      else if (progress.l == "hold") { progress.l = 2 }
      else if (progress.l == "final_hold") { progress.l = 3 }

      if (progress.l == device.StepLabels.RAMP) { progress.label = "ramp"; }
      else if (progress.l == device.StepLabels.HOLD) { progress.label = "hold"; }
      else if (progress.l == device.StepLabels.FINAL_HOLD) { progress.label = "final_hold"; }

      this.progress = progress;
      if (!this.protocol) return;
      this.elapsedTime = Util.humanTime(this.progress.e/1000);
      this.stepElapsedSec = Math.round(this.progress.d / 1000);
      // Calculate estimated remaining time
      const remaining = this.getRemainingTimeMs(this.protocol, progress);
      this.elapsedTime = this.progress.e;
      this.elapsedTimeLabel = Util.humanTime(this.progress.e / 1000);
      this.remainingTimeLabel = Util.humanTime(remaining/1000);
      this.totalTimeLabel = Util.humanTime((remaining+this.progress.e)/1000);
      this.remainingTime = remaining
      this.totalTime = (remaining+this.progress.e)
      this.plateTemp = (this.progress) ? Math.round(this.progress.p) : "-";
      this.airTemp = (this.progress) ? Math.round(this.progress.a) : "-";
    },
    pause () { device.pause(); },
    resume () { device.resume(); },
    cancel () { 
      if (window.confirm("Are you sure you want to cancel the experiment?")) {
        device.cancel(); 
        appState.home();
      }
    },
    finish () { 
      device.finish();
    },
    home () { 
      this.closeModals();
      this.$emit('home');
      // appState.home();
    },
    result () {
      this.closeModals();
    },
    closeModals () {
      this.$bvModal.hide('finish-modal');
      this.$bvModal.hide('result-modal');
    },
    dl (format) {
      device.downloadCurrentExperimentLog(format);
    }
    
  }
}
</script>
