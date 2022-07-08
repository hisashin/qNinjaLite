from mqtt_packet import MQTTProtocol

mqtt = MQTTProtocol()

class MQTTClient:
    def __init__ (self, network, keep_alive=60):
        self.last_ping = 0
        self.keep_alive = keep_alive
        self.on_message = None
        self.network = network
    def set_on_message (self, func):
        self.on_message = func

    # TODO Depends on platforms
    # TODO Handle errors
    def _time_ms (self):
        return self.network.time_ms()
    def _recv(self, length):
        return self.network.read(length)
    def _write(self, packet):
        self.network.write(packet)
        self.last_ping = self._time_ms()

    def connect(self):
        # packet = mqtt.connect(keep_alive=self.keep_alive, will_topic="bye", will_payload="Goodbye", will_retain=True, will_qos=0)
        packet = mqtt.connect(keep_alive=self.keep_alive)
        print("Connecting 1")
        self._write(packet)
        try:
            print("Connecting 2") # Stop
            resp = bytearray(self._recv(4))
            print("Connecting 3")
            self.debug(mqtt.connack_read(resp))
        except Exception as e:
            print("Connecting 4")
            print(e)
            

    def subscribe (self, topic_filter, requested_qos=0):
        packet = mqtt.subscribe(topic_filter, requested_qos=requested_qos)
        try:
            self._write(packet)
            resp = bytearray(self._recv(1024))
        except Exception as e:
            print(e)

    def debug (self, message):
        print(message)

    def ping (self):
        try:
            packet = mqtt.pingreq()
            self._write(packet)
            resp = self._recv(1024)
            res = mqtt.pingres_read(bytearray(resp))
            self.debug(res)
        except:
            print("ping timeout")

    #  Returns True when expecting server response
    def _process_response(self, res_raw):
        print("process")
        if res_raw == None or len(res_raw) < 1:
            return False
        print("process 1")
        res = bytearray(res_raw)
        packet_type = res[0] >> 4
        if packet_type == MQTTProtocol.CONNACK:
            return False
        if packet_type == MQTTProtocol.PUBLISH:
            result = mqtt.publish_read(res)
            self.debug(result)
            if result["valid"]:
                self.on_message(result)
                if result["qos"] == 1:
                    self.debug("Sending PUBACK.")
                    packet = mqtt.puback(message_identifier=result["packet_identifier"])
                    self._write(packet)
                    self.debug("PUBACK sent.")
                    return False
                if result["qos"] == 2:
                    self.debug("Sending PUBREC.")
                    packet = mqtt.pubrec(message_identifier=result["packet_identifier"])
                    self._write(packet)
                    self.debug("PUBREC sent.")
                    return True
        if packet_type == MQTTProtocol.PUBACK:
            # Nothong to do
            return False
        # PUBREC -> PUBREL
        if packet_type == MQTTProtocol.PUBREC:
            result = mqtt.pubrec_read(res)
            self.debug("Sending PUBREL.")
            packet = mqtt.pubrel(message_identifier=result["packet_identifier"])
            self._write(packet)
            self.debug("PUBREL sent.")
            return False
        # PUBREL -> PUBCOMP
        if packet_type == MQTTProtocol.PUBREL:
            result = mqtt.pubrel_read(res)
            self.debug("Sending PUBCOMP.")
            packet = mqtt.pubcomp(message_identifier=result["packet_identifier"])
            self._write(packet)
            self.debug("PUBCOMP sent.")
            return False
        if packet_type == MQTTProtocol.PUBCOMP:
            return False
        if packet_type == MQTTProtocol.SUBACK:
            return False
        if packet_type == MQTTProtocol.UNSUBACK:
            return False


    def loop(self):
        next = True
        while next:
            next = False
            try:
                resp = self._recv(1024)
                try:
                    next = self._process_response(resp)
                    if (next):
                        print("Continue...")
                except Exception as e:
                    print(e)
            except Exception as e:
                # Timeout
                # print(e)
                pass
        if self._time_ms() - self.last_ping > (self.keep_alive - 10) * 1000:
            self.ping()

    def publish (self, topic, message, qos=0):
        # TODO manages messages with qos > 0
        packet = mqtt.publish(topic, message, qos=qos)
        self._write(packet)

