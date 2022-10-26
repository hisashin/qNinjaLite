"use strict";
// Manage topic tree
class TopicManager {
  constructor () {
    this.topicRoot = {
      subTopic: "",
      topics: {},
      subscribers: [],
    };
    this.idMap = {};
    this.maxSubId = 0;
    this.count = 0;
  }
  add (topic, item) {
    const path = topic.split("/");
    const subId = this._issueSubId();
    let node = this.topicRoot;
    if (topic.length > 0) {
      for (let depth = 0; depth < path.length; depth++) {
        const subTopic = path[depth];
        if (node.topics[subTopic]) {
          node = node.topics[subTopic];
        } else {
          const newNode = {
            topic: subTopic,
            topics: {},
            subscribers: []
          };
          node.topics[subTopic] = newNode;
          node = newNode;
        }
      }
    }
    node.subscribers.push({id:subId, item:item});
    this.idMap[subId] = topic;
    this.count ++;
    return subId;
  }
  _find (path, depth, node, targets) {
    const subTopic = path[depth];
    if (depth == path.length) {
        node.subscribers.forEach((subscriber)=>{
            // console.log("Push %s", subscriber)
            targets.push(subscriber);
        });

    } else {
        if (node.topics[subTopic]) {
            // Next level
            this._find(path, depth+1, node.topics[subTopic], targets);
        }
        if (node.topics["+"]) {
            // Next level
            this._find(path, depth+1, node.topics["+"], targets);
        }
        if (node.topics["#"]) {
            // Skip some levels
            for (let i=depth; i<=path.length; i++) {
                this._find(path, i, node.topics["#"], targets);
            }
        }
    }
  }
  find (topic) {
    const path = topic.split("/");
    let node = this.topicRoot;
    let targets = []
    this._find(path, 0, this.topicRoot, targets);
    return targets.map(v=>v.item);
  }
  remove (id) {
    const topic = this.idMap[id];
    if (topic == null) {
      console.log("Topic %d not found.",id);
      return false;
    }
    const path = topic.split("/");
    const objPath = [];
    let node = this.topicRoot;
    objPath.push(node);
    for (let depth = 0; depth < path.length; depth++) {
      const subTopic = path[depth];
      if (node.topics[subTopic]) {
        node = node.topics[subTopic];
        objPath.push(node);
      } else {
        break;
      }
    }
    delete this.idMap[id];
    // Find node to remove
    for (let i=0; i<node.subscribers.length; i++) {
      if (node.subscribers[i].id == id) {
        node.subscribers.splice(i, 1);
        break;
      }
    }
    // Remove empty node
    let depth = objPath.length - 1;
    let subTopic = null;
    while (depth >= 0) {
      const node = objPath[depth];
      if (subTopic != null) {
        delete node.topics[subTopic];
      }
      subTopic = node.topic;
      const isEmpty = node.subscribers.length==0 && Object.keys(node.topics).length == 0;
      if (!isEmpty) {
        break;
      }
      depth -= 1;
    }
    this.count --;
    return true;
  }
  _issueSubId () {
    this.maxSubId ++;
    return this.maxSubId;
  }
}
module.exports = TopicManager;

