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
          Stop the device
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
            v-for="(step, index) in protocol.steps" :key="index" 
            v-bind:class="{ 'progress-meter__item--notyet': progress.step<index,'progress-meter__item--current': progress.step==index,'progress-meter__item--done': progress.step>index }">
            <span class="progress-meter__item progress-meter__label">{{ step.temp }}℃ {{ step.duration }}s</span>
            <div
              v-if="progress.step==index" class="progress-meter__item progress-meter__detail">{{ progress.step_label }}</div>
          </li>
          <li
            class="progress-meter__item"
            v-bind:class="{ 'progress-meter__item--current': progress.step_label=='final_hold', 'progress-meter__item--notyet': progress.step_label!='final_hold' }">
            <span class="progress-meter__item progress-meter__label">Finished</span>
          </li>
        </ul>
      </div>
      
      <div class="progress-monitor__row">
        <div class="temperature-monitor">
          <div class="temperature-monitor__label">
            Plate
            <div class="temperature-monitor__value">{{ plateTemp }} 
            <template 
              v-if="step!=null && step.temp!=null">/{{step.temp}}℃</template>
            </div>
          </div>
          <div class="temperature-monitor__meter">
            <meter class="temperature-monitor__meter-body"
              min="20"
              max="100"
              :value="progress.plate"></meter>
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
        Air={{ progress.air }}
        
      </div>
      <div class="progress-monitor__row" v-if="progress.step_label!='final_hold'">
        <div class="time-monitor">
          <div class="time-monitor__elapsed">{{ elapsedTimeLabel }} </div>
          <div class="time-monitor__estimated"> / {{ totalTimeLabel }}</div>
          <div class="time-monitor__remaining"> ({{ remainingTimeLabel }} left)</div>
          <meter class="time-monitor__meter" 
            min="0"
            :max="remainingTime+elapsedTime"
            :value="progress.elapsed"></meter>
          
          <template v-if="deviceState != null">
          <button class="time-monitor__button" v-if="deviceState.pause_available" @click="pause">Pause</button>
          <button class="time-monitor__button" v-if="deviceState.resume_available" @click="resume">Resume</button>
          <button class="time-monitor__button" v-if="deviceState.cancel_available" @click="cancel">Cancel</button>
          <button class="time-monitor__button" v-if="deviceState.finish_available" @click="finish">Finish</button>
          <!--<button class="time-monitor__button" v-if="deviceState.start_available" @click="start">Start</button>-->
          </template>
          
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
    }
  },
  computed: {
  },
  created: function () {
    device.subscribe(device.experiment_data_topic("progress"), (topic, data, id)=>{
      this.applyProgress(data);
    });
    device.protocol.observe((protocol)=>{
      console.log("ProgressMonitor.protocol set.")
      this.protocol = protocol;
    });
    console.log("Observe device.deviceState");
    device.deviceState.observe((state)=>{
      console.log("device.deviceState")
      this.closeModals(); 
      if (this.deviceState && state)
        console.log([this.deviceState.label, state.label, 
          this.deviceState.finish_available, state.finish_available, this.deviceState.has_experiment, state.has_experiment])
      if (this.deviceState && !this.deviceState.finish_available && state.finish_available) {
        this.$bvModal.show('finish-modal');
      }
      if (this.deviceState && this.deviceState.has_experiment && !state.has_experiment) {
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
      if (!protocol.steps) {
        throw "Protocol has no steps."
      }
      if (!progress) {
        throw "Progress is null."
      }
      if (progress.step_label == "final_hold") {
        // Experiment is complete
        return 0;
      }
      protocol.steps.forEach((step, index)=>{
        if (progress.step  == index) {
          // Current step
          if (progress.step_label == "ramp") {
            // Ramp time
            time += this.getEstimatedTransitionTimeMs(progress.plate, step.temp);
            // Hold time
            time += step.duration * 1000;
          } else if (progress.step_label == "hold") {
            time += (step.duration * 1000 - progress.step_elapsed)
          } else {
            throw "Unknown progress.step_label value: " + progress.step_label;

          }
        } else if (progress.step  < index) {
          // Ramp time
          time += this.getEstimatedTransitionTimeMs(protocol.steps[index-1].temp, step.temp);
          // Hold time
          time += step.duration * 1000;
        }
      });
      if (isNaN(time)) {
        console.log("NaN???")
      }
      return time;

    },
    applyProgress (progress) {
      this.progress = progress;
      if (!this.protocol) return;
      this.elapsedTime = Util.humanTime(this.progress.elapsed/1000);
      // Calculate estimated remaining time
      const remaining = this.getRemainingTimeMs(this.protocol, progress);
      this.elapsedTime = this.progress.elapsed;
      this.elapsedTimeLabel = Util.humanTime(this.progress.elapsed / 1000);
      this.remainingTimeLabel = Util.humanTime(remaining/1000);
      this.totalTimeLabel = Util.humanTime((remaining+this.progress.elapsed)/1000);
      this.remainingTime = remaining
      this.totalTime = (remaining+this.progress.elapsed)
      this.plateTemp = (this.progress) ? Math.round(this.progress.plate) : "-";
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
      appState.home();
    },
    result () {
      this.closeModals(); 
      appState.revealDetailLatestExperiment(this);
    },
    closeModals () {
      this.$bvModal.hide('finish-modal');
      this.$bvModal.hide('result-modal');
    }
    
  }
}
</script>
