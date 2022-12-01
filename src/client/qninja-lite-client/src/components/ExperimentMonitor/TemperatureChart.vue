<template>
  <div class="temperature-chart">
    <div>
      <canvas class="temperature-chart__graph"
        ref="canvas"
        width="800"
        height="400"
        style="width:800px,height:400px"
      />
    </div>
    <b-button class="mr-1 btn-sm"
      @click.stop="head()">
      &lt;&lt;
    </b-button>
    <b-button class="mr-1 btn-sm"
      @click.stop="pan(-1)">
      &lt;
    </b-button>
    <b-button class="mr-1 btn-sm"
      @click.stop="zoom(-1)">
      -
    </b-button>
    <b-button class="mr-1 btn-sm"
      @click.stop="zoom(1)">
      +
    </b-button>
    <b-button class="mr-1 btn-sm"
      @click.stop="pan(1)">
      &gt;
    </b-button>
    <b-button class="mr-1 btn-sm"
      @click.stop="tail()">
      &gt;&gt;
    </b-button>
  </div>
</template>

<script>
import Graph from "../../lib_shared/Graph.js";
// const TIME_RANGES = [10000, 1000, 500, 250, 120, 60];
const TIME_RANGES = [200, 100, 80, 60, 40, 20]; // Narrow ranges for debugging
const PADDING = 10;
let timeRangeSec = TIME_RANGES[3];

export default {
  data() {
    return {
      channelPlate: null,
      zoomLevel: 3,
      timeOffset:0
    }
  },
  created: function () {
    this.channelPlate = null;
    this.zoomLevel = 3;
    this.dataRange = {};
    this.dataRangePrev = {};
    this.chartRange = {};
    this.minTime = 0;
    this.maxTime = 120;
    this.timeOffset = 0;
  },
  mounted: function () {
    this.minTime = 0;
    this.maxTime = 120;
    this.timeOffset = 0;
    const CONVERSION_FUNCTION = (obj)=>{return { "x":obj.t/1000.0, "y":obj.v } };
    let graph = new Graph(this.$refs.canvas);
    this.channelPlate = graph.addChannel({index:0, label:"Plate"}).addSubChannel({type:"line"});
    this.channelPlate.conversionFunction = CONVERSION_FUNCTION;
    graph.setMinMaxY(20, 120);
    this.graph = graph;
  },
  methods: {
    clear: function () {
      console.log("TODO TemperatureChart.clear()");
      this.dataRange = {};
      this.dataRangePrev = {};
      this.graph.clearData();
    },
    
    updateGraphRange: function () {
      if (this.dataRange.min===undefined || this.dataRange.max===undefined) return;
      let chartTimeRange = {
        min:0,
        max:0
      };
      if (this.dataRange.max - this.dataRange.min < timeRangeSec) {
        // Data range is fully covered
        chartTimeRange.min = this.dataRange.min;
        chartTimeRange.max = this.dataRange.min + timeRangeSec + PADDING;
      } else {
        // Auto-scroll is suppressed if the graph is showing intermediate range.
        const diff = timeRangeSec + PADDING + this.timeOffset - this.dataRangePrev.max;
        if (diff > 0) {
          this.timeOffset = Math.max(this.dataRange.max - (timeRangeSec + PADDING - diff));
        }
        chartTimeRange.min = this.timeOffset;
        chartTimeRange.max = this.timeOffset + timeRangeSec + PADDING;
      }
      /*
      if (chartTimeRange.min < 0) {
        console.error("CHART ERROR");
        console.log("this.dataRangePrev")
        console.log(this.dataRangePrev)
        console.log("chartTimeRange")
        console.log(chartTimeRange)
        console.log("this.dataRange")
        console.log(this.dataRange)
        console.log("this.timeOffset")
        console.log(this.timeOffset)
        console.log("timeRangeSec")
        console.log(timeRangeSec)

      }
      */
      this.graph.setMinMaxX(chartTimeRange.min, chartTimeRange.max);

    },
    add: function(timestamp, plate) {
      this.channelPlate.addData({t:timestamp, v:plate});
      const timestampSec = timestamp/1000;
      this.updateDataRange(timestampSec);
      this.maxTime = Math.max(this.maxTime, timestampSec);
      this.updateGraphRange();
      this.graph.update();
    },
    set: function (data) {
      
      this.minTime = 0;
      this.maxTime = 120;
      this.timeOffset = 0;
      this.graph.clearData();
      for (let i=0; i<data.length; i++) {
        const p = data[i]
        const time = p.e; 
        const timeSec = time/1000;
        const plate = p.p;
        if (time == null || plate == null) {
          continue;
        }

        this.updateDataRange(timeSec);
        this.channelPlate.addData({t:time, v:plate});
        this.maxTime = Math.max(this.maxTime, timeSec);
      }
      this.tail();
    },
    updateDataRange : function (timeSec) {
      if (this.dataRange.min!== undefined && this.dataRange.min!== undefined) {
        this.dataRangePrev = {
          min: this.dataRange.min,
          max: this.dataRange.max
        };
        this.dataRange.max = Math.max(this.dataRange.max, timeSec);
        this.dataRange.min = Math.min(this.dataRange.min, timeSec);
      } else {
        // Graph is empty
        this.dataRange = {
          min: timeSec,
          max: timeSec
        };
        this.dataRangePrev = {
          min: timeSec,
          max: timeSec
        };
      }
    },
    zoom: function (step) {
      const tail = this.timeOffset + timeRangeSec;
      this.zoomLevel = Math.min(Math.max(0, this.zoomLevel + step), TIME_RANGES.length - 1);
      timeRangeSec = TIME_RANGES[this.zoomLevel];
      this.timeOffset = tail - timeRangeSec;
      this.updateGraphRange();
      this.graph.update();
    },
    pan: function (step) {
      this.timeOffset = Math.max(0, this.timeOffset + step * timeRangeSec);
      this.updateGraphRange();
      this.graph.update();
      
    },
    head: function () {
      if (this.dataRange.min === undefined) {
        this.timeOffset = 0;
        return;
      }
      this.timeOffset = this.dataRange.min;
      this.updateGraphRange();
      this.graph.update();
    },
    tail: function () {
      if (this.dataRange.min === undefined) {
        this.timeOffset = 0;
        return;
      }
      this.timeOffset = Math.max(this.dataRange.min, this.dataRange.max - timeRangeSec);
      this.updateGraphRange();
      this.graph.update();
    }
  }
}
</script>