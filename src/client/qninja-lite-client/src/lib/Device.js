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
      this.onopen();
    };
    this.ws.onclose = (e) => {
      this.onclose(e);
    };
    this.ws.onerror = (e) => {
      this.onerror(e);
    };
  }
  _apiEndpoint() {
    return "ws://" + WS_API_HOST + ":" + WS_API_PORT + "/";
  }
  _issueReqId() {

  }
  publish(topic, data) {
    this.ws.send(JSON.stringify(data));
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
    this.connectionStatus = new ObservableValue(Connection.DISCONNECTED);
    this.connectionStatus.observe((value)=>{
      console.log("NetworkAWSMQTT.connectionStatus changed.");
      console.log(JSON.stringify(value))
      if (value == Connection.DEVICE_CONNECTED) {
        this.onConnectedToDevice();
      }
    });
    setInterval(()=>{ this.keepDeviceConnection() }, 2000);
  }
  _dataTopic() {
    return "dt/ninja/" + this.thingId;
  }
  _commandTopic() {
    return "cmd/ninja/" + this.thingId;
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
      this.connectionStatus.set(Connection.SERVER_CONNECTED);
      const dataTopicFilter = this._dataTopic() + "/#";
      const commandTopicFilter = this._commandTopic() + "/#";
      console.log("NetworkAWSMQTT.client.connect subscribing...")
      this.client.subscribe(commandTopicFilter, (e) => {
        console.log("NetworkWebsocket.client.subscribe cmd OK %s", commandTopicFilter)
        console.log("NetworkAWSMQTT.client.on.connect subscribing dt...")
        this.client.subscribe(dataTopicFilter, (e) => {
          console.log("NetworkWebsocket.client.subscribe dt callback %s", dataTopicFilter)
          // Ready to pub & sub ping 
          const pingTopic = this._commandTopic() + "/ping-client";
          this.publish(pingTopic, EMPTY_MSG);
        });
      });
    });
    this.client.on('reconnect', (e) => {
      console.log("### NetworkAWSMQTT.client.reconnect")
      if (this.onopen)
        this.onopen();
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
        // console.log("### NetworkAWSMQTT.onMessage topic=%s, message=%s", topic, message);
        if (topic == this._commandTopic() + "/ping-device") {
          // Ping on device boot
          console.log("Connection established!!! (client first)");
          const pingResTopic = this._commandTopic() + "/ping-device-res";
          this.publish(pingResTopic, EMPTY_MSG);
          this.connectionStatus.set(Connection.DEVICE_CONNECTED);

        }
        if (topic == this._commandTopic() + "/ping-client-res") {
          // Respons to my ping on boot
          console.log("Connection established!!! (device first)");
          this.connectionStatus.set(Connection.DEVICE_CONNECTED);
        }
        this.onmessage(topic, obj);
      } catch (e) {
        console.error("NetworkAWSMQTT.client.message Malformed message");
        console.error(e);
      }
    });
  }
  keepDeviceConnection () {
    const now = new Date();
    if (this.lastMessage == null || now.getTime() - this.lastMessage.getTime()  < 10 * 1000) {
      return;
    }
    // 30s from last message
    const pingTopic = this._commandTopic() + "/ping-client";
    if (this.client && (this.lastPing == null || now.getTime()- this.lastPing.getTime()  > 30 * 1000)) {
      console.log("PING")
      this.publish(pingTopic, EMPTY_MSG);
      this.lastPing = now;
    }
    if (now.getTime() - this.lastMessage.getTime()  > 30 * 1000) {
      // No response for 30s
        console.log("### NetworkAWSMQTT device went offline.");
        this.connectionStatus.set(Connection.CONNECTED_SERVER);
    }
  }
  onConnectedToDevice() {
    if (this.onopen) {
      setTimeout(() => {
        console.log("### NetworkAWSMQTT.client.on.open")
        this.onopen();
      }, 5000)
    }
  }
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
  }
  init(config) {
    this.config = config;
    this.experiment.observe((experiment) => {
      if (experiment) {
        this.protocol.set(experiment.protocol);
      }
    });
    this.subscribe(this.device_data_topic_filter("state"), (topic, data, id) => {
      console.log("Device.js deviceState updated.")
      console.log(data)
      this.deviceState.set(data);
    });
    this.network.onopen = () => {
      console.log('network.onopen');
      device.publish(device.device_command_topic("req-state"), {}, (res) => {
        console.log("Received req-state response.");
        console.log(res.g)
        this.deviceState.set(res.g);
      });
    };
    console.log("subscribing protocol/query-res")
    this.subscribe(this.device_command_topic_filter("protocol/query-res"), (topic, data, id) => {
      console.log("Received protocol/query-res")
      console.log(data.response.Items)
      if (this.loadProtocolsCallback) {
        this.loadProtocolsCallback(data.response.Items)
      }
      this.loadProtocolsCallback = null;
    });
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
  _thing_id() {
    return this.thingId;
  }
  _experiment_id() {
    return this.experimentId;
  }
  experiment_data_topic(category) {
    return "dt/ninja/" + this._thing_id() + "/experiment/+/" + category
  }
  experiment_data_topic_filter(category) {
    return "dt/ninja/+/experiment/+/" + category
  }
  device_data_topic(category) {
    return "dt/ninja/" + this._thing_id() + "/" + category
  }
  device_data_topic_filter(category) {
    return "dt/ninja/+/" + category
  }
  device_command_topic(command) {
    return "cmd/ninja/" + this._thing_id() + "/" + command
  }
  device_command_topic_filter(command) {
    return "cmd/ninja/+/" + command
  }
  experiment_command_topic(command) {
    return "cmd/ninja/" + this._thing_id() + "/experiment/" + this._experiment_id() + "/" + command
  }
  experiment_command_topic_filter(command) {
    return "cmd/ninja/+/experiment/" + this._experiment_id() + "/" + command
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
    let obj = {
      "topic": topic,
      "data": data
    };
    console.log("### Device.publish topic=%s", topic)
    if (responseHandler) {
      const reqId = this._issueReqId();
      this.reqIdMap[reqId] = responseHandler;
      data.q = reqId;
      const expectedResponse = { q: reqId, data:{} };
    }
    data.sender = SENDER_ID;
    this.network.publish(topic, data);
  }

  _issueReqId() {
    this.maxReqId++;
    return this.maxReqId;
  }

  /* API */
  connect(connectionInfo) {
    this.thingId = connectionInfo.thingId;
    console.log("Device.connect()")
    try {
      this.network.connect(connectionInfo);
    } catch (ex) {
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

  _issueExperimentId() {
    return "" + new Date().getTime();
  }
  start(protocol, callback) {
    const experimentId = this._issueExperimentId();
    const obj = { i: experimentId, p: protocol };
    console.log(obj);
    this.publish(this.device_command_topic("start"), obj, () => {
      this.experimentId = experimentId;
      console.log("Experiment start command was accepted! this.experimentId=" + this.experimentId)
      callback()
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
    console.log("LOAD_PROTOCOLS");
    /*
    Util.requestData(
      "https://api-ninja.hisa.dev/protocol?uid=uk&pretty=true",
      // "data/protocols.json",
       null, "GET", (data)=>{
      callback(data)
    }, onError);
    */
    this.loadProtocolsCallback = callback;
    this.publish(this.device_command_topic("protocol/query"), {}, (res) => {
      console.log(res)
    });
  }

}
const device = new Device();
device.Connection = Connection;
module.exports = device;
