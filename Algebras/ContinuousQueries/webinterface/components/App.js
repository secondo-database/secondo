

class App extends React.Component {
  state =  {
    location: "login", // login, list, build
    errors: [],
    user: {
      hash: "",
      email: "",
      strategy: "",
      tupledescr: "",
      tupleextracts: [],
    },
  }

  getLocationComponent = () => {
    if (this.state.location == "list") {
      return  <List 
                user={this.state.user}
                onError={this.handleError}
                onLocationChange={this.handlelocationChange} 
              />
    }

    if ((this.state.location == "build") && (this.state.user.strategy == "loop"))
      return <BuildLoop 
              user={this.state.user}
              onError={this.handleError}
              onLocationChange={this.handlelocationChange} 
            />

    if ((this.state.location == "build") && (this.state.user.strategy == "join")) 
      return <BuildJoin 
              user={this.state.user}
              onError={this.handleError}
              onLocationChange={this.handlelocationChange} 
            />

    return  <Login 
              loginFailed={this.state.loginFailed} 
              onLocationChange={this.handlelocationChange} 
              onRegister={this.handleRegister}
              onLogin={this.handleLogin}
            />
  }

  dismissError = (key) => {
    const errors = this.state.errors.filter(e => e.key !== key);
    
    this.setState({errors});
  }

  handleError = (reason, badge) => {
    if (!badge) badge = "info";

    console.log(badge + ": ", reason);

    let errors = this.state.errors;
    errors.push({
      key: reason + "_" + sha1("" + new Date() + Math.random()),
      reason: reason,
      badge: badge
    });
  
    this.setState({errors});
  }

  handleUserAuth = (hash, email, type) => {
    var apicall = sspglobals.apiurl
      + "?cmd="   + "userauth"
      + "&hash="  + hash
      + "&email=" + email
      + "&type="  + type;
    
    var apisettings = {
      method: "GET",
      cache: "no-store",
      mode: "cors"
    }
    
    // call the server to create this user
    fetch(apicall, apisettings).then((response) => {
      if (response.ok)
          return response.json();
      else
          this.handleError("Error while using the API!", "danger");
          throw new Error("Error while using the API!");
      })
      .then((json) => {
          // returns an error
          if (json.result[0][0] == "error") {
            
            this.handleError(json.result[0][1], "warning");

            let user = {...this.state.user};
                user.hash = "";
                user.email = "";
                user.strategy = "";
                user.tupledescr = "";
                user.tupleextracts = [];

            
            this.setState({user});
          }

          // returns the command, userhash and tupledescr
          if (json.result[0][0] == "userauth") {
            const regex = /(\(([a-zA-Z0-9]*) (string|text|int|real|bool)\))/gm;
            let tupleextracts = [];
            let m;

            while ((m = regex.exec(json.result[0][2])) !== null) {
              if (m.index === regex.lastIndex) regex.lastIndex++;
              tupleextracts.push({name: m[2], type: m[3]});
            }

            let user = {...this.state.user};
                user.hash = hash;
                user.email = email;
                user.tupledescr = json.result[0][2];
                user.strategy = json.result[0][3];
                user.tupleextracts = tupleextracts;
            
            const location = "list";

            this.setState({user, location});
          }
      })
      .catch((err) => {
          this.handleError("Fetch catched an error!", "danger");
          console.log(err);
      });
  }

  handleLogin = (email, password) => {
    if ((email.search("@") == -1) || (email.search(".") == -1)) {
      this.handleError("Not an email address!", "warning");
      return;
    }

    if (password.trim().length == 0) {
      this.handleError("Passwords can't be empty!", "warning");
      return;
    }

    this.handleUserAuth(sha1(email+password), email, "login");
  }
  
  handleRegister = (email, password1, password2) => {
    if ((email.search("@") == -1) || (email.search(".") == -1)) {
      this.handleError("Not an email address!", "warning");
      return;
    }

    if (password1 != password2) {
      this.handleError("Passwords are not the same!", "warning");
      return;
    }

    if (password1.trim().length == 0) {
      alert("Passwords can't be empty!");
      return;
    }

    this.handleUserAuth(sha1(email+password1), email, "register");
  }

  handlelocationChange = target => {
    this.setState({ location: target});
  }

  render() {
    return (
      <div className="container">
        { this.state.errors.map(error => <AlertComponent
            key = {error.key}
            id = {error.key}
            reason = {error.reason}
            badge = {error.badge}
            dismissError = {this.dismissError}
          />) }
        { this.getLocationComponent() }
      </div>
    )
  }
}


class AlertComponent extends React.Component {
  render() {
    const {id, reason, badge, dismissError} = this.props;

    return (
      <div
        id={id}
        className={"m-2 alert text-center alert-"+badge} 
        role="alert"
      >
          {reason}

          <button onClick={() => dismissError(id)} type="button" className="close" data-dismiss="alert" aria-label="Close">
            <span aria-hidden="true">&times;</span>
        </button>
      </div>
    )
  }
}