from mqtt_packet import MQTTProtocol

mqtt = MQTTProtocol()

class MQTTClient:
    def __init__ (self, network, keep_alive=60, client_id="", buff_size=4096):
        self.last_ping = 0
        self.keep_alive = keep_alive
        self.on_message = None
        self.network = network
        self.client_id = client_id
        self.buff_size = buff_size
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
        print("MQTT Connecting...")
        packet = mqtt.connect(keep_alive=self.keep_alive, client_id=self.client_id)
        self._write(packet)
        try:
            r = self._recv(4)
            if r == None:
                print("Received none")
                return
            resp = bytearray(r)
            self.debug(mqtt.connack_read(resp))
        except Exception as e:
            print("Connecting ERR")
            print(e)

    def subscribe (self, topic_filter, requested_qos=0):
        packet = mqtt.subscribe(topic_filter, requested_qos=requested_qos)
        try:
            self._write(packet)
            resp = bytearray(self._recv(self.buff_size))
        except Exception as e:
            print("NO Sub Resp")

    def debug (self, message):
        if False:
            print(message)

    def ping (self):
        try:
            packet = mqtt.pingreq()
            self._write(packet)
            resp = self._recv(self.buff_size)
            res = mqtt.pingres_read(bytearray(resp))
            self.debug(res)
        except:
            print("ping timeout")

    #  Returns True when expecting server response
    def _process_response(self, res_raw):
        if res_raw == None or len(res_raw) < 1:
            return False
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
                    packet = mqtt.puback(message_identifier=result["packet_identifier"])
                    self._write(packet)
                    self.debug("PUBACK sent.")
                    return False
                if result["qos"] == 2:
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
                resp = self._recv(self.buff_size)
                try:
                    next = self._process_response(resp)
                    if (next):
                        print("Continue...")
                except Exception as e:
                    print(e)
            except Exception as e:
                # Timeout
                print("Loop error")
                print(e)
                pass
        if self._time_ms() - self.last_ping > (self.keep_alive - 10) * 1000:
            self.ping()

    def publish (self, topic, message, qos=0):
        # TODO manages messages with qos > 0
        packet = mqtt.publish(topic, message, qos=qos)
        self._write(packet)
