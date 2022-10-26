<!-- Inside "detail-card__body" -->
<template>
    <div v-if="protocol">
        <h2>Protocol</h2>
        <div>
          Final hold temp: {{ protocol.final_hold_temp }}
        </div>
        <ul>
            <template v-for="(step, index) in protocol.steps">
                <li :key="index">
                    Step {{ index }}  temp={{step.temp}}, duration={{step.duration}}, 
                    data_collection={{step.data_collection}}, data_colection_interval={{step.data_colection_interval}}
                </li>
            </template>
        </ul>
    </div>
</template>

<script>
import appState from "../lib/AppState.js";
import Constants from "../lib/constants.js";


let LABEL_MAP = {};
LABEL_MAP[Constants.StageType.HOLD] = "Hold";
LABEL_MAP[Constants.StageType.QPCR] = "qPCR";
LABEL_MAP[Constants.StageType.MELT_CURVE] = "Melt Curve";
LABEL_MAP[Constants.StageType.PCR] = "Normal PCR";
  
export default {
  name: 'ProtocolDetail',
  
  components:{},
  props: ['protocol'],
  data() {
    return {
      id:""
    }
  },
  created: function () {
  },
  methods: {
    back: function () {
      appState.backPanel();
    }
  }
}
</script>
