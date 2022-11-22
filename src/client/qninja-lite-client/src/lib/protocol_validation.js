"use strict";

const Validator = require("./validator.js");

const PLATE_TEMP_MAX = 100;
const PLATE_TEMP_MIN = 20;

const RULE_DATA_COLLECTION = {
  ramp_end: { type:"boolean", required:false },
  hold_end: { type:"boolean", required:false },
  ramp_continuous: { type:"boolean", required:false },
  measurement_interval_temp: { type:"number", required:false, min:0.1, max: 10.0 }
};
const RULE_STEP = {
  temp: { type:"number", required:true, min:PLATE_TEMP_MIN, max:PLATE_TEMP_MAX },
  duration: { type:"number", required:true, min:0, max: 24 * 60 * 60 * 1000 },
  data_collection: { type:"number", required:true, min:0, max:1 },
  data_collection_interval: { type:"number", required:false, min:1, max:60 * 3 },
  _func: ()=>{}
};
const RULE_PROTOCOL = {
  name: { type:"string", required:true, min_length:1, max_length:255 },
  final_hold_temp: { type:"number", required:false, min:PLATE_TEMP_MIN, max:PLATE_TEMP_MAX },
  steps: { type:"array",required:true,  min_length:1, max_length:8, rule:RULE_STEP },
  _func: ()=>{}
};
module.exports = new Validator(RULE_PROTOCOL);

