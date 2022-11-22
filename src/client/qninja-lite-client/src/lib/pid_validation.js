const Validator = require("./validator.js");

const validateCoverage = (ranges)=>{
    let minValue = null;
    while (true) {
        const found = ranges.find((range)=>{
            return ((minValue == null && range.min_value == null) || 
            (minValue != null && range.min_value != null && range.min_value <= minValue));
        });
        if (!found) {
            return {result:false, message:"Not all temperature ranges are covered"}
        } else {
            if (found.max_value == null) {
                return {result:true, message:"OK"};
            }
            minValue = found.max_value;
            // console.log("minValue=%f", minValue)
        }
    }
};
const RULE_RANGE = {
    kp: { type:"number", required:true, min:0 },
    ki: { type:"number", required:true, min:0 },
    kd: { type:"number", required:true, min:0 },
    max_value: { type:"number", required:false },
    min_value: { type:"number", required:false },
    _func: (data, errors, path)=>{
        if (typeof data.max_value=="number" && typeof data.min_value=="number" && data.max_value <= data.min_value) {
            errors.push({message:"max_value should be greater than min_value", path:path});
        }
    }
};
const RULE_PID = {
    consts: {
        type:"array", 
        required:true, 
        min_length:1, 
        max_length:8,
        rule:RULE_RANGE
    },
    "_func": (data,errors,path)=>{
        /*
        console.log("Func1")
        console.log(data)
        */
        if (!Array.isArray(data.consts)) {
            // console.log("Not an array")
            return;
        }
        const result = validateCoverage(data.consts);
        if (!result.result) {
            errors.push({message:result.message, path:path});
        }
    }
};

const validator = new Validator(RULE_PID);
module.exports = validator;


/*
const validatePIDConstants = (str)=>{
    let ranges = null;
    try {
        ranges = JSON.parse(str);
    } catch (e) {
        return {result:false, message:"Invalid format"};
    }
    const validationResult = validator.validate(ranges);
    return validationResult;
};
const CONSTS = `[
    {"kp":0.3, "ki":0, "kd":0, "min_value":80},
    {"kp":0.3, "ki":0, "kd":0, "max_value":55},
    {"kp":0.3, "ki":0, "kd":0, "min_value":55, "max_value":80}
  ]`;
const CONSTS_NOT_ARRAY = `{"kp":0.3, "ki":0, "kd":0}`;
const CONSTS_INVALID_COVERAGE = `[
    {"kp":0.3, "ki":0, "kd":0, "min_value":80, "max_value":90},
    {"kp":0.3, "ki":0, "kd":0, "max_value":55},
    {"kp":0.3, "ki":0, "kd":0, "min_value":58, "max_value":80}
  ]`;
const CONSTS_MALFORMED = `___[
    {"kp":0.3, "ki":0, "kd":0, "min_value":80},
    {"kp":0.3, "ki":0, "kd":0, "max_value":55},
    {"kp":0.3, "ki":0, "kd":0, "min_value":58, "max_value":80}
  ]`;
console.log("CONSTS")
console.log(validatePIDConstants(CONSTS));
console.log("CONSTS_NOT_ARRAY")
console.log(validatePIDConstants(CONSTS_NOT_ARRAY));
console.log("CONSTS_INVALID_COVERAGE")
console.log(validatePIDConstants(CONSTS_INVALID_COVERAGE));
console.log("CONSTS_MALFORMED")
console.log(validatePIDConstants(CONSTS_MALFORMED));
*/