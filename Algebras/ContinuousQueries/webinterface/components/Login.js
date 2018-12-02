

class Login extends React.Component {
  state = {
    loginFailed: false,
    loginEmail: "1.@",
    loginPassword: "1",
    registerEmail: "",
    registerPassword: "",
    registerPasswordRepeat: "",
  }

  handleChange = evt => {
    this.setState({ [evt.target.id]: evt.target.value });
  }

  render() {
    return (
      <div>
        <div className="card" style={{ width: 36 + "rem", margin: "0 auto" }}>
          <div className="card-body">
            <h5 className="card-title">Login</h5>
            <h6 className="card-subtitle mb-2 text-muted">Enter your email and password to login. Then you can see and alter your already commited queries.</h6>
            <p className="card-text">
              <input onChange={this.handleChange} type="email" className="form-control" id="loginEmail" placeholder="E-Mail" />
            </p>
            <p className="card-text">
              <input onChange={this.handleChange} type="password" className="form-control" id="loginPassword" placeholder="Password" />
            </p>
            <button
              onClick={() => this.props.onLogin(
                this.state.loginEmail,
                this.state.loginPassword
              )}
              className="btn btn-secondary"
              style={{ width: 100 + "%" }}
            >LOGIN</button>
          </div>
        </div>

        <hr className="hr-text" data-content="OR" />

        <div className="card" style={{ width: 36 + "rem", margin: "0 auto" }}>
          <div className="card-body">
            <h5 className="card-title">Create User</h5>
            <h6 className="card-subtitle mb-2 text-muted">Enter an email address and a password to create a new user and commit queries.</h6>
            <p className="card-text">
              <input onChange={this.handleChange} type="email" className="form-control" id="registerEmail" placeholder="E-Mail" />
            </p>
            <p className="card-text">
              <input onChange={this.handleChange} type="password" className="form-control" id="registerPassword" placeholder="Password" />
            </p>
            <p className="card-text">
              <input onChange={this.handleChange} type="password" className="form-control" id="registerPasswordRepeat" placeholder="Repeat Password" />
            </p>
            <button
              onClick={() => this.props.onRegister(
                this.state.registerEmail,
                this.state.registerPassword,
                this.state.registerPasswordRepeat
              )}
              className="btn btn-secondary"
              style={{ width: 100 + "%" }}
            >REGISTER</button>
          </div>
        </div>

      </div>
    )
  }
}


