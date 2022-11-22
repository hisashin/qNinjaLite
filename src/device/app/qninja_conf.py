import os
import json
import gc

conf_dir_path = "conf"
code2d_file_name = "code2d.json"
conf_file_name = "conf.json"
conf_file_path = conf_dir_path + "/" + conf_file_name
code2d_file_path = conf_dir_path + "/" + code2d_file_name
class Conf:
    # Properties: thing_id(thingName), code2d_matrix(QR matrix), code2d_type("provisioning"/"cycler")
    def __init__(self):
        self.conf_obj = {}
        self.code2d_obj = {}
        if conf_file_name in os.listdir(conf_dir_path):
            try:
                print(conf_file_path)
                with open(conf_file_path, 'rb') as f:
                    self.conf_obj = json.loads(f.read())
                    print("Loaded conf file")   
            except:
                print("Failed to load conf file")
        else:
            print("File not found")
    def load_code2d(self):
        if code2d_file_name in os.listdir(conf_dir_path):
            try:
                print(code2d_file_path)
                with open(code2d_file_path, 'rb') as f:
                    self.code2d_obj = json.loads(f.read())
                    print("Loaded conf2d")   
            except:
                print("Failed to load code2d file")
        else:
            print("Code2d file not found")

    def set_thing_id(self, thing_id):
        self.conf_obj["thing_id"] = thing_id
    def thing_id(self):
        return self.conf_obj.get("thing_id", None)

    def set_pid(self, pid):
        self.conf_obj["pid"] = pid
    def pid(self):
        return self.conf_obj.get("pid", {})
    
    def set_code2d_matrix(self, matrix):
        self.code2d_obj["code2d_matrix"] = matrix
    def code2d_matrix(self):
        return self.code2d_obj.get("code2d_matrix", None)

    def set_code2d_type(self, t):
        self.code2d_obj["code2d_type"] = t
    def code2d_type(self):
        return self.code2d_obj.get("code2d_type", None)

    def clean_code2d(self):
        self.code2d_obj = None
        gc.collect()

    def save(self):
        print("conf.save")
        print(self.conf_obj)
        try:
            with open(conf_file_path, 'w+') as f:
                f.write(json.dumps(self.conf_obj))
                f.close()
                print("Saved conf")
        except:
            print("Failed to save conf file")
    def save_code2d(self):
        print("conf.save_code2d")
        print(self.code2d_obj)
        try:
            with open(code2d_file_path, 'w+') as f:
                f.write(json.dumps(self.code2d_obj))
                f.close()
                print("Saved code2d")
        except:
            print("Failed to save code2d file")

conf = Conf()