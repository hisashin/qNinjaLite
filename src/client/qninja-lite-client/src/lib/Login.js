class LoginManager {
  constructor () {
    this.loginInfo = {
      username:"",
      password:"",
      autologin:false
    };
    try {
      this.loginInfo.username = localStorage.getItem("username");
      this.loginInfo.password = localStorage.getItem("password");
      this.loginInfo.autologin = localStorage.getItem("autologin")=="true";
    } catch (e) {
      console.log("Failed to load username & password");
    }
  }
  isFilled () {
    return this.loginInfo.username != null && this.loginInfo.username.length > 0 && 
    this.loginInfo.password != null && this.loginInfo.password.length > 0;
  }
  getLoginInfo () {
    return this.loginInfo;
  }
  setLoginInfo (loginInfo) {
    console.log(loginInfo)
    this.loginInfo = loginInfo;
    try {
      localStorage.setItem("username", this.loginInfo.username);
      localStorage.setItem("password", this.loginInfo.password);
      localStorage.setItem("autologin", this.loginInfo.autologin?"true":"false");
    } catch (e) {
      console.log("Failed to save username & password");
    }
  }
}
const loginManager = new LoginManager();
module.exports = loginManager;