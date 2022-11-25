const Validator = require("./validator.js");

const validateCoverage = (ranges)=>{
    let minValue = null;
    while (true) {
        const found = ranges.find((range)=>{
            return ((minValue == null && range.n == null) || 
            (minValue != null && range.n != null && range.n <= minValue));
        });
        if (!found) {
            return {result:false, message:"Not all temperature ranges are covered"}
        } else {
            if (found.x == null) {
                return {result:true, message:"OK"};
            }
            minValue = found.x;
            // console.log("minValue=%f", minValue)
        }
    }
};
const RULE_RANGE = {
    p: { type:"number", required:true, min:0 }, // Kp
    i: { type:"number", required:true, min:0 }, // Ki
    d: { type:"number", required:true, min:0 }, // Kd
    x: { type:"number", required:false }, // Max value
    n: { type:"number", required:false }, // Min value
    _func: (data, errors, path)=>{
        if ((typeof data.x)=="number" && (typeof data.n)=="number" && data.x <= data.n) {
            errors.push({message:"Max value should be greater than min value", path:path});
        }
    }
};
const RULE_PID = {
    c: { // Consts
        type:"array", 
        required:true, 
        min_length:1, 
        max_length:8,
        rule:RULE_RANGE
    },
    "_func": (data,errors,path)=>{
        if (!Array.isArray(data.c)) {
            return;
        }
        const result = validateCoverage(data.c);
        if (!result.result) {
            errors.push({message:result.message, path:path});
        }
    }
};

const validator = new Validator(RULE_PID);
module.exports = validator;
