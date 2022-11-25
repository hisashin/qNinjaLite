"use strict";

const Validator = require("./validator.js");

const PLATE_TEMP_MAX = 100;
const PLATE_TEMP_MIN = 20;

const RULE_STEP = {
  t: { type:"number", required:true, min:PLATE_TEMP_MIN, max:PLATE_TEMP_MAX }, // Temperature
  d: { type:"number", required:true, min:0, max: 24 * 60 * 60 * 1000 }, // Duration
  c: { type:"number", required:true, min:0, max:1 }, // Data collection flag
  i: { type:"number", required:false, min:1, max:60 * 3 }, // Data collection interval
  _func: ()=>{}
};
const RULE_PROTOCOL = {
  /*name: { type:"string", required:true, min_length:1, max_length:255 },*/
  h: { type:"number", required:false, min:PLATE_TEMP_MIN, max:PLATE_TEMP_MAX },
  s: { type:"array",required:true,  min_length:1, max_length:8, rule:RULE_STEP },
  _func: ()=>{}
};
module.exports = new Validator(RULE_PROTOCOL);

