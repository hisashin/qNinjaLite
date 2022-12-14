"use strict";
const AWS_HOST = "wss://a2n3d0j9ie2pgn-ats.iot.ap-northeast-1.amazonaws.com:443/mqtt?x-amz-customauthorizer-name=iot_custom_authorizer";
// const TopicManager = require("/Users/maripo/git/Ninja-qPCR/src/qpcr/lib/topic_manager.js");
const TopicManager = require("../lib/topic_manager.js");
const Util = require("../lib/Util.js");
const mqtt = require("./mqtt.js");
const SENDER_ID = "543210"
const EMPTY_MSG = { sender: SENDER_ID };
const MAX_RETRY_INTERVAL = 60;
// const WS_API_HOST = "192.168.100.103"

class ObservableValue {
  constructor(value) {
    this._value = value;
    this.observers = [];
    this.maxId = 0;
  }
  set(value) {
    const changed = this._value != value;
    this._value = value;
    if (changed) {
      this.observers.forEach((observer) => { observer.f(this._value) });
    }
  }
  get() {
    return this._value;
  }
  observe(observer) {
    observer(this._value);
    const id = this.maxId;
    this.maxId++;
    this.observers.push({ f: observer, id: id });
    setImmediate(observer(this._value))
    return id;
  }
  removeObserver(id) {
    this.observers = this.observers.filter((obs) => { return obs.id != id });
  }
}
const Connection = {
  INITIAL: { server:{connected:false, message:"Disconnected"}, device:{connected:false, message:"-"} },
  DISCONNECTED: { server:{connected:false, message:"Disconnected"}, device:{connected:false, message:"-"} },
  SERVER_CONNECTED: { server:{connected:true, message:"Connected"}, device:{connected:false, message:"Offline"} },
  DEVICE_CONNECTED: {  server:{connected:true, message:"Connected"}, device:{connected:true, message:"Online"}  },
  ERROR: { server:{connected:false, message:"Connection error"}, device:{connected:false, message:"-"}  }
};
const StepLabels = {
  RAMP: 1,
  HOLD: 2,
  FINAL_HOLD: 3
};

const WS_API_PORT = "8888";
const WS_API_HOST = "192.168.1.13"
class NetworkWebsocket {
  constructor() {
    this.ws = null;
    /* Handlers */
    this.onopen = null;
    this.onclose = null;
    this.onerror = null;
    this.onmessage = null;
  }
  connect(connectionInfo) {
    console.log("NetworkWebsocket.connect");
    this.ws = new WebSocket(this._apiEndpoint());
    this.ws.onmessage = (e) => {
      console.log("NetworkWebsocket.onmessage");
      const obj = JSON.parse(e.data);
      const topic = obj.topic;
      const data = obj.data;
      this.onmessage(topic, data);
    };
    this.ws.onopen = () => {
      console.log("NetworkWebsocket.onopen");
      if (this.onopen)
        this.onopen();
    };
    this.ws.onclose = (e) => {
      this.onclose(e);
    };
    this.ws.onerror = (e) => {
      this.onerror(e);
      console.error(e);
      this.connectionStatus.set(this.Connection.ERROR);
    };
  }
  _apiEndpoint() {
    return "ws://" + WS_API_HOST + ":" + WS_API_PORT + "/";
  }
  _issueReqId() {

  }
  publish(topic, data) {
    this.ws.send(JSON.stringify({topic:topic,data:data}));
  }
}
class NetworkAWSMQTT {
  constructor() {
    this.ws = null;
    /* Handlers */
    console.log("### NetworkAWSMQTT.constructor");
    this.onopen = null;
    this.onclose = null;
    this.onerror = null;
    this.onmessage = null;
    this.client = null;
    this.thingId = "";
    this.retryInterval = 10;
    this.lastMessage = null;
    this.connectionStatus = new ObservableValue(Connection.INITIAL);
  }
  _dataTopic() {
    return "dt/ninja/" + this.thingId;
  }
  _commandTopic() {
    return "cmd/ninja/" + this.thingId;
  }
  _awsTopic() {
    return "aws/ninja/" + this.thingId;
  }
  connect(connectionInfo) {
    console.log("### NetworkAWSMQTT.connect");
    console.log(JSON.stringify(connectionInfo))
    this.thingId = connectionInfo.thingId;
    let host = AWS_HOST + "&thing=" + connectionInfo.thingId;
    let options = {
      rejectUnauthorized: false,
      username: connectionInfo.username,
      password: connectionInfo.password,
      clientId: connectionInfo.username + "_" + new Date().getTime()%10000
    };
    this.client = mqtt.connect(host, options);
    this.client.on('error', (e) => {
      console.log("### NetworkAWSMQTT.client.error")
      this.onerror(e);
    });
    this.client.on('connect', (e) => {
      this.retryInterval = 1;
      const dataTopicFilter = this._dataTopic() + "/#";
      const commandTopicFilter = this._commandTopic() + "/#";
      const awsTopicFilter = this._awsTopic() + "/#";
      console.log("NetworkAWSMQTT.client.connect subscribing...")
      this.client.subscribe(commandTopicFilter, (e) => {
        console.log("NetworkWebsocket.client.subscribe cmd OK %s", commandTopicFilter)
        console.log("NetworkAWSMQTT.client.on.connect subscribing dt...")
        this.client.subscribe(dataTopicFilter, (e) => {
          console.log("NetworkWebsocket.client.subscribe dt callback %s", dataTopicFilter)
          this.client.subscribe(awsTopicFilter, (e) => {
            console.log("NetworkWebsocket.client.subscribe aws callback %s", awsTopicFilter)
            // Ready to pub & sub ping 
            this.connectionStatus.set(Connection.SERVER_CONNECTED);
            const pingTopic = this._awsTopic() + "/req";
            this.publish(pingTopic, EMPTY_MSG);
            if (this.onopen) {
              this.onopen();
            }
          });
        });
      });
    });
    this.client.on('reconnect', (e) => {
      console.log("### NetworkAWSMQTT.client.reconnect")
      /*
      if (this.onopen)
        this.onopen();
        */
    });
    this.client.on('close', (e) => {
      // Called in case of disconnection.
      console.log("this.client.on.close")
      this.connectionStatus.set(Connection.DISCONNECTED);
      this.lastMessage = null;
      this.client.end()
      this.client = null;
      if (this.onclose) {
        this.onclose();
      }
    });
    this.client.on('disconnect', (e) => {
      // Not called in case of getting offline.
      console.log("### NetworkAWSMQTT.client.disconnect");
    });
    
    this.client.on('message', (topic, message) => {
      try {
        const obj = JSON.parse(message.toString());
        if (obj.sender == SENDER_ID) {
          return;
        }
        this.lastMessage = new Date();
        /*
        if (topic == this._awsTopic() + "/req-res") {
          const isOnline = obj.response.Item.o;
          if (isOnline) {
            this.connectionStatus.set(Connection.DEVICE_CONNECTED);
          }
        }
        */
        this.onmessage(topic, obj);
      } catch (e) {
        console.error("NetworkAWSMQTT.client.message Malformed message");
        console.error(e);
      }
    });
  }
  /*
  onConnectedToDevice() {
    if (this.onopen) {
      setTimeout(() => {
        console.log("### NetworkAWSMQTT.client.on.open")
        this.onopen();
      }, 5000)
    }
  }
  */
  publish(topic, data) {
    console.log("### NetworkAWSMQTT.publish", topic)
    this.client.publish(topic, JSON.stringify(data));
  }

}
class Device {
  constructor() {
    this.network = new NetworkAWSMQTT();
    // this.network = new NetworkWebsocket(); // WebSocket object

    this.topicManager = new TopicManager();
    this.config = {};

    this.deviceState = new ObservableValue(null);
    this.experiment = new ObservableValue(null);
    this.protocol = new ObservableValue(null);
    this.reqIdMap = {};
    this.maxReqId = 0;
    this.experimentId = "0";
    this.thingId = "";
    this.uid = "uk";
  }
  init(config) {
    this.config = config;
    this.experiment.observe((experiment) => {
      if (experiment) {
        this.protocol.set(experiment.protocol);
      }
    });
    this.subscribe(this.aws_command_topic_filter("req-res"), (topic, data,id)=>{
      const stateObj = data.response.Item;
      const isOnline = stateObj.o;
      console.log("STATE UPDATED " + JSON.stringify(stateObj))
      if (isOnline) {
        this.network.connectionStatus.set(Connection.DEVICE_CONNECTED);
      } else {
        this.network.connectionStatus.set(Connection.SERVER_CONNECTED);
      }
      this.deviceState.set(stateObj);
    })
    // TODO remove this block after v1 test
    console.log("PING DEVICE topic=" + this.device_command_topic_filter("ping-device"))
    this.subscribe(this.device_command_topic_filter("ping-device"), (topic, data, id) => {
      this.network.connectionStatus.set(Connection.SERVER_CONNECTED)
      console.log("PING_DEVICE RECEIVED")
      setTimeout(()=>{
        this.publish(this.device_command_topic("req-state"), {}, (data)=>{
          console.log("INITIAL_REQ_STATE");
          console.log(data)
          this.deviceState.set(data);
        });
      }, 1000);
    });
    this.subscribe(this.device_data_topic_filter("state"), (topic, data, id) => {
      console.log("STATE Device.js deviceState updated. topic=" + topic)
      console.log(data)
      this.deviceState.set(data);
    });
    this.subscribe(this.aws_command_topic_filter("presence"), (topic, presence)=>{
      console.log("PRESENCE ", presence);
      if (presence.o == false) {
        this.network.connectionStatus.set(Connection.SERVER_CONNECTED)
      }
      if (presence.o == true) {
        this.network.connectionStatus.set(Connection.DEVICE_CONNECTED)
      }
    });
    this.network.onopen = () => {
      console.log('network.onopen');
      device.publish(device.device_command_topic("req-state"), {}, (res) => {
        console.log("INITIAL_STATE");
        console.log(res.g)
        this.deviceState.set(res.g);
      });
    };
    this.network.onmessage = (topic, data) => {
      // Process response to cmd
      if (data.q) {
        if (this.reqIdMap[data.q]) {
          try {
            this.reqIdMap[data.q](data);
          } catch (e) {
            console.error(e)
          } finally {
            delete this.reqIdMap[data.q];
          }
        }
      }
      // Call subscribers
      const targets = this.topicManager.find(topic);
      targets.forEach((subscriber) => {
        try {
          subscriber.handler(topic, data);
        } catch (e) {
          console.log(e);
          console.trace();
        }
      });
    };
    this.network.onclose = (e) => {
      //
    };
    this.network.onerror = (e) => {
      // console.error(e);
    };
  }

  /* Event bus proxy functionalities */
  subscribe(topic, handler/* (topic, data)=>{} */) {
    console.log("SUBSCRIBE %s", topic)
    if (typeof (topic) != 'string') {
      throw "EventBus.subscribe topic should be <string> type.";
    }
    if (typeof (handler) != 'function') {
      throw "EventBus.subscribe handler should be <handler> type.";
    }
    const subscription = {
      handler: handler,
      topic: topic
    };
    const subId = this.topicManager.add(topic, subscription);
    return subId;
  }
  unsubscribe(subId) {
    this.topicManager.remove(subId);
    console.log("Device.UNsubscribe count=%d", this.topicManager.count);
  }
  publish(topic, data, responseHandler) {
    console.log("### Device.publish topic=%s", topic)
    if (responseHandler) {
      const reqId = this._issueReqId();
      this.reqIdMap[reqId] = responseHandler;
      data.q = reqId;
    }
    console.log(data)
    data.sender = SENDER_ID;
    this.network.publish(topic, data);
  }

  /* API */
  connect(connectionInfo) {
    this.thingId = connectionInfo.thingId;
    console.log("Device.connect()")
    try {
      this.network.connect(connectionInfo);
    } catch (ex) {
      console.error(ex)
      return;
    }
  }

  /* Experiment Control */
  pause() {
    this.publish(this.experiment_command_topic("pause"), {});
  }
  resume() {
    this.publish(this.experiment_command_topic("resume"), {});
  }
  cancel() {
    this.publish(this.experiment_command_topic("cancel"), {});
  }
  finish() {
    this.publish(this.experiment_command_topic("finish"), {});
  }

  start(protocol, callback, onError) {
    this.publish(this.aws_command_topic("experiment/req-newid"), {}, (res) => {
      console.log(res)
      if (!res.response.error) {
        const experimentId = res.response.eid;
        const obj = { i: experimentId, p: protocol };
        console.log(obj);
        this.publish(this.device_command_topic("start"), obj, () => {
          this.experimentId = experimentId;
          console.log("Experiment start command was accepted! this.experimentId=" + this.experimentId)
          callback()
        });
      } else {
        if (!onError) {
          onError();
        }
      }
    });
  }
  confPID (obj, callback) {
    console.log(obj);
    this.publish(this.device_command_topic("pid"), obj, () => {
      console.log("PID const updated")
      callback()
    });
  }
  loadProtocols (callback, onError) {
    this.publish(this.aws_command_topic("protocol/req-query"), {tid:this.getThingID()}, (res) => {
      callback(res.response.Items)
    });
  }
  loadExperiments (callback, onError) {
    this.publish(this.aws_command_topic("experiment/req-query"), {}, (res) => {
      callback(res.response.Items)
    });
  }
  loadExperimentProgress (eid, callback, onError) {
    this.publish(this.aws_command_topic("experiment/" + eid + "/progress/req-query"), {}, (res) => {
      callback(res.response.Items)
    });
  }
  loadExperimentFluo (eid, callback, onError) {
    this.publish(this.aws_command_topic("experiment/" + eid + "/fluo/req-query"), {}, (res) => {
      callback(res.response.Items)
    });
  }
  saveProtocol (protocolItem, callback, onError) {
    // aws/ninja/qninja_1667831590228/protocol/req-put
    protocolItem.tid = device.getThingID();
    this.publish(this.aws_command_topic("protocol/req-put"),  protocolItem, (res) => {
      callback(res)
    });
   console.log(JSON.stringify(protocolItem))
  }
  deleteProtocol (protocolItem, callback, onError) {
    // Set d=true to delete
    let item = JSON.parse(JSON.stringify(protocolItem));
    item.d = true;
    console.log("deleteProtocol")
    console.log(item)
    this.publish(this.aws_command_topic("protocol/req-put"),  item, (res) => {
      callback(res)
    });
  }
  downloadCurrentExperimentLog (fileType) {
    //aws_experiment_command_topic
    console.log("download " + this.aws_experiment_command_topic("req"))
    this.publish(this.aws_experiment_command_topic("req"), {}, 
    (result) => {
      console.log("download req " + JSON.stringify(result))
      if (!result.response.Item.urlZipFriendlyCsv) {
        this.publish(this.aws_experiment_command_topic("req-backup"), {}, (result)=>{
          console.log("download req-backup " + JSON.stringify(result))
          
          // {"q":9,"tid":"qninja_1667831590228","eid":"dfG9WRBIYh6aA","teid":"qninja_1667831590228_dfG9WRBIYh6aA","urlZipFriendlyJson":"https://q.hisa.dev/backup/thing/qninja_1667831590228/experiment/qninja_1667831590228_dfG9WRBIYh6aA_json.gz","urlZipFriendlyCsv":"https://q.hisa.dev/backup/thing/qninja_1667831590228/experiment/qninja_1667831590228_dfG9WRBIYh6aA_csv.gz","gen":false}
          const url = (fileType=="csv")?result.urlZipFriendlyCsv:result.urlZipFriendlyJson;
          window.open(url);
        });

      }
    });
  }
  downloadExperimentLog (experiment, fileType) {
    if (experiment.urlZipFriendlyCsv && fileType == "csv") {
      window.open(experiment.urlZipFriendlyCsv)
    }
    else if (experiment.urlZipFriendlyJson && fileType == "json") {
      window.open(experiment.urlZipFriendlyJson)
    } else {
      // aws/ninja/<thing-id>/experiment/<exp-id>/req-backup
      this.publish(this.aws_command_topic("experiment/" + experiment.eid + "/req-backup"), {}, (result) => {
        console.log(result)
        experiment.urlZipFriendlyCsv = result.urlZipFriendlyCsv
        experiment.urlZipFriendlyJson = result.urlZipFriendlyJson
        let url = (fileType=="csv")? result.urlZipFriendlyCsv : result.urlZipFriendlyJson;
        window.open(url);
      }, ()=>{
      }
      );

    }
  }
  getThingID () {
    return this.thingId;
  }
  _experiment_id () {
    return this.experimentId;
  }
  experiment_data_topic (category) {
    return "dt/ninja/" + this.getThingID() + "/experiment/+/" + category
  }
  experiment_data_topic_filter (category) {
    return "dt/ninja/+/experiment/+/" + category
  }
  device_data_topic (category) {
    return "dt/ninja/" + this.getThingID() + "/" + category
  }
  device_data_topic_filter (category) {
    return "dt/ninja/+/" + category
  }
  device_command_topic (command) {
    return "cmd/ninja/" + this.getThingID() + "/" + command
  }
  device_command_topic_filter (command) {
    return "cmd/ninja/+/" + command
  }
  aws_command_topic (command) {
    return "aws/ninja/" + this.getThingID() + "/" + command
  }
  aws_experiment_command_topic (command) {
    return "aws/ninja/" + this.getThingID() + "/experiment/" + this._experiment_id() + "/" + command;
  }
  aws_command_topic_filter (command) {
    return "aws/ninja/+/" + command
  }
  experiment_command_topic (command) {
    return "cmd/ninja/" + this.getThingID() + "/experiment/" + this._experiment_id() + "/" + command
  }
  experiment_command_topic_filter (command) {
    return "cmd/ninja/+/experiment/" + this._experiment_id() + "/" + command
  }
  _issueReqId() {
    this.maxReqId++;
    return this.maxReqId;
  }
}
const device = new Device();
device.Connection = Connection;
device.StepLabels = StepLabels;
module.exports = device;
