import binascii
class MQTTProtocol:
    def __init__(self):
        self.packet_identifier_max = 1

    def _int_to_bytes (self, int_val, length):
        a = [0x00] * length
        for i in range(length):
            a[length - i - 1] = (int_val >> (8 * i)) & 0xFF
        return bytearray(a)

    def _bytes_to_int (self, bytes):
        val = 0
        for i in range(len(bytes)):
            val |= bytes[len(bytes)-i-1] << (8*i)
        return val

    def connect(self, keep_alive = 60, user_name=None, password=None, clean_session=True,
        client_id="", will_flag=False, will_topic="", will_payload="", will_retain=False, will_qos=0):
        header = bytearray(b"\x10\x00") # Connect command + Length
        body = bytearray(b"")
        body += b"\x00\x04" # Protocol Length
        body += "MQTT" # Protocol
        body += b"\x04" # Protocol version
        # TODO detailed flags
        conn_flag_index = len(body)
        body += bytearray([0x00]) # Connect flags
        assert keep_alive <= 0xFFFF, "Keep Alive should be <= 0xFFFF"
        body += self._int_to_bytes(keep_alive, 2) # Keep-alive (60)

        conn_flag = 0x00 # 0x02
        if clean_session:
            conn_flag |= 0x01 << 1
        # Client Identifier
        body += self._int_to_bytes(len(client_id), 2)
        body += client_id # Client ID
        assert will_retain == False or will_flag, "If the Will Flag is set to 0, then Will Retain MUST be set to 0"
        assert will_flag or will_qos == 0, "If the Will Flag is set to 0, then the Will QoS MUST be set to 0 (0x00)"
        assert will_flag == False or (will_qos == 0 or will_qos == 1 or will_qos == 2), "If the Will Flag is set to 1, the value of Will QoS can be 0 (0x00), 1 (0x01), or 2 (0x02). It MUST NOT be 3 (0x03)"
        if will_flag:
            conn_flag |= 0x01 << 2
            # Will Topic
            body += self._int_to_bytes(len(will_topic), 2)
            body += will_topic
            # Will Payload
            body += self._int_to_bytes(len(will_payload), 2)
            body += will_payload
            conn_flag |= (will_qos & 0x03) << 3
        
        # User Name
        if user_name:
            conn_flag |= 0x01 << 7
            body += self._int_to_bytes(len(user_name), 2)
            body += user_name
        # Password
        if password:
            conn_flag |= 0x01 << 6
            body += self._int_to_bytes(len(password), 2)
            body += password
        if will_retain:
            conn_flag |= 0x01 << 5

        body[conn_flag_index] = conn_flag
        header[1] = len(body) & 0xFF
        packet = header + body
        return packet

    def connack_read(self, packet):
        valid = len(packet) == 4 and packet[0] == 0x20 and packet[1] == 0x02
        result = {
            'valid': valid, #  Validity of the packet
            'session_present': False,
            'connect_return_code': 0x00,
            'connection_accepted': False
        }
        if valid:
            result['session_present_flag'] = packet[2] & 0x01 == 0x01
            # Connect Return Code
            # 0x00 Connection Accepted
            # 0x01 Connection Refused, unacceptable protocol version
            # 0x02 Connection Refused, identifier rejecte
            # 0x03 Connection Refused, Server unavailabled
            # 0x04 Connection Refused, bad user name or password
            # 0x05 Connection Refused, not authorized
            connect_return_code = packet[3]
            result['connect_return_code'] = connect_return_code
            result['connection_accepted'] = connect_return_code == 0x00
        return result

    def disconnect(self):
        return bytearray(b"\xe0\x00")

    def subscribe(self, topic_filter, message_identifier=1, requested_qos=0):
        header = bytearray(b"\x82\x00")
        body = bytearray("")
        body += self._int_to_bytes(message_identifier, 2)
        body += self._int_to_bytes(len(topic_filter), 2)
        body += topic_filter
        body += self._int_to_bytes(requested_qos, 1)

        header[1] = len(body) & 0xFF
        packet = header + body
        return packet

    def suback_read(self, packet):
        valid = len(packet) == 5 and packet[0] == 0x90 and packet[1] == 0x03
        result = {
            'valid': valid, #  Validity of the packet
            'message_identifier': 0,
            'success' : False,
            'qos' : 0
        }
        if valid:
            print(packet[2:4])
            result['message_identifier'] = self._bytes_to_int(packet[2:4])
            if packet[4] <= 0x02:
                result['success'] = True
                result['qos'] = packet[4]
        return result

        
    def _decode_length (self, bytes, offset):
        i = 0
        val = 0
        has_next = True
        while (has_next):
            b = bytes[offset+i]
            val += (b & 0x7F) << (i * 7)
            i += 1
            has_next = b & 0x80
        return (val, i)

    def _encode_length (self, length):
        bytes = []
        while length > 0:
            v = 0x7F & length
            if length >= 0x80:
                v |= 0x80
            bytes.append(v)
            length = length >> 7
        return bytearray(bytes)

    def publish(self, topic, message, dup=False, qos=0, retain=False, packet_identifier=0):
        header0 = 0x03 << 4
        packet_identifier_to_send = 0
        if packet_identifier > 0:
            packet_identifier_to_send = packet_identifier
        else:
            packet_identifier_to_send = self.packet_identifier_max
            self.packet_identifier_max += 1
        if dup:
            header0 |= (0x01 << 3)
        header0 |= (qos & 0x03) << 1
        if retain:
            header0 |= (0x01 << 0)
        header = bytearray([header0])
        body = bytearray("")
        body += self._int_to_bytes(len(topic), 2)
        body += topic # UTF-8 encoded string
        if qos > 0:
            body += self._int_to_bytes(packet_identifier_to_send, 2)
        body += message
        header += self._encode_length(len(body))
        packet = header + body
        return packet

    def publish_read(self, packet):
        (body_length, length_byte_count) = self._decode_length(packet, 1)
        print("publish_read")
        print([body_length, length_byte_count], len(packet))
        valid = len(packet) >= 2 and packet[0] >> 4 == 0x03 and body_length == len(packet) - length_byte_count - 1
        result = {
            'valid': valid, #  Validity of the packet
            'qos': 0,
            'dup': False,
            'retain': False,
            'topic': '',
            'message': '',
            'packet_identifier': None
        }
        if valid:
            result['dup'] = (packet[0]>> 3) & 0x01 == 0x01
            result['retain'] = (packet[0]>> 0) & 0x01 == 0x01
            qos = 0x03 & packet[0] >> 1
            result['qos'] = qos
            offset = 1 + length_byte_count
            topic_length = self._bytes_to_int(packet[offset:offset+2])
            offset += 2
            result['topic'] = packet[offset:offset+topic_length].decode("utf-8")
            print(result['topic'])
            offset += topic_length
            if qos > 0:
                result['packet_identifier'] = self._bytes_to_int(packet[offset:offset+2])
                offset += 2
            result['message'] = packet[offset:len(packet)].decode("utf-8")
        return result

    def puback(self, message_identifier):
        packet = bytearray(b"\x40\x02")
        packet += self._int_to_bytes(message_identifier, 2)
        return packet

    def pubrec(self, message_identifier):
        packet = bytearray(b"\x50\x02")
        packet += self._int_to_bytes(message_identifier, 2)
        return packet

    def pubrec_read(self, packet):
        valid = len(packet) == 4 and packet[0] >> 4 == 0x05
        result = { 'valid' : valid, 'packet_identifier' : 0 }
        if valid:
            result['packet_identifier'] = self._bytes_to_int(packet[2:4])
        return result

    def pubrel(self, message_identifier):
        packet = bytearray(b"\x62\x02")
        packet += self._int_to_bytes(message_identifier, 2)
        return packet

    def pubrel_read(self, packet):
        valid = len(packet) == 4 and packet[0] >> 4 == 0x06
        result = { 'valid' : valid, 'packet_identifier' : 0 }
        if valid:
            result['packet_identifier'] = self._bytes_to_int(packet[2:4])
        return result

    def pubcomp(self, message_identifier):
        packet = bytearray(b"\x70\x02")
        packet += self._int_to_bytes(message_identifier, 2)
        return packet

    def pingreq (self):
        return bytearray(b"\xc0\x00")


    def pingres_read (self, packet):
        return {"valid": packet == b"\xd0\x00" }


def compare(packet, expected):
    print(binascii.hexlify(expected))
    print(binascii.hexlify(packet))
    debug = ""
    for i in range(max(len(packet), len(expected))):
        if i < len(packet) and i < len(expected) and packet[i]==expected[i]:
            debug += "  "
        else:
            debug += "^^"
    print(debug)

def debug():
    mqtt = MQTTProtocol()
    conn = mqtt.connect(keep_alive=60, will_topic="bye", will_payload="Goodbye", will_retain=True, will_qos=2)
    conn_expected = bytearray(b"\x10\x1a\x00\x04\x4d\x51\x54\x54\x04\x36\x00\x3c\x00\x00\x00\x03\x62\x79\x65\x00\x07\x47\x6f\x6f\x64\x62\x79\x65")
    compare(conn, conn_expected)

    pub_expected = bytearray(b"\x32\x0c\x00\x04\x68\x6f\x67\x65\x00\x01\x66\x75\x67\x61")
    pub = mqtt.publish(topic="hoge", message="fuga", qos=1, packet_identifier=1)
    compare(pub, pub_expected)

    disconnect_expected = bytearray(b"\xe0\x00")
    disconnect = mqtt.disconnect()
    compare(disconnect, disconnect_expected)

    connack = mqtt.connack_read(bytearray(b"\x20\x02\x00\x00"))
    print(connack)

    ping_expected = bytearray(b"\xc0\x00")
    ping = mqtt.ping()
    compare(ping, ping_expected)

    publish_read = mqtt.publish_read(pub_expected)
    print(publish_read)


    sub_expected = bytearray(b"\x82\x09\x00\x01\x00\x04\x68\x6f\x67\x65\x00")
    #sub = mqtt.subscribe(topic_filter="hoge",message_identifier=1, requested_qos=0)
    compare(sub, sub_expected)

    suback = mqtt.suback_read(bytearray(b"\x90\x03\x00\x01\x00"))
    print(suback)

    # ping "\xc0\x00"
    # ping response "\xd0\x00"

    # sub ack "\x90\x03\x00\x01\x00"

MQTTProtocol.CONNECT = 1
MQTTProtocol.CONNACK = 2
MQTTProtocol.PUBLISH = 3
MQTTProtocol.PUBACK = 4
MQTTProtocol.PUBREC = 5
MQTTProtocol.PUBREL = 6
MQTTProtocol.PUBCOMP = 7
MQTTProtocol.SUBSCRIBE = 8
MQTTProtocol.SUBACK = 9
MQTTProtocol.UNSUBSCRIBE = 10
MQTTProtocol.UNSUBACK = 11
MQTTProtocol.PINGREQ = 12
MQTTProtocol.PINGRESP = 13
MQTTProtocol.DISCONNECT = 14