

class List extends React.Component {
    state = {
      queries: []
    }

    getAllQueries = () => {
        var apicall = sspglobals.apiurl
            + "?cmd=" + "getqueries"
            + "&hash=" + this.props.user.hash;

        fetch(apicall).then((response) => {
            if (response.ok)
                return response.json();
            else
                throw new Error("Error beim API Aufruf!");
            })
        .then((json) => {
            if (json.result.length == 0) {
                this.setState({queries: []});
                return;
            }

            if (json.result[0][0] == "error") {
                alert(json.result[0][1]);
                this.setState({queries: []});
                return;
            }

            if (json.result[0][0] == "getqueries") {
                const rr = json.result.map(arr =>{return {id: arr[1], func: arr[2]}});
                this.setState({queries: rr});
            }
        })
        .catch((err) => {
            console.log(err);
        });
    }

    componentDidMount = () => {
        this.getAllQueries();
    }

    renderTable = () => {
        if (this.state.queries.length == 0) return <p>No queries yet...</p>

        return (
            <table className="table table-striped">
                <thead>
                    <tr>
                        <th scope="col">id</th>
                        <th scope="col">function</th>
                        <th scope="col" className="text-right">action</th>
                    </tr>
                </thead>

                <tbody>
                    {this.state.queries.map(q => <QueryRow key={q.id} id={q.id} func={q.func} />)}
                </tbody>
            </table>
        )
    }
  
    render() {
      return (
        <React.Fragment>
            <h1>List</h1>
            <h3>Hello {this.props.user.email}</h3>
            <h4>{this.props.user.tupledescr}</h4>

            <br/><hr/><br/>

            {this.renderTable()}

            <button 
                onClick={() => this.props.onLocationChange("build")}
                className="btn btn-primary float-right"
            >Add New Query</button>

        </React.Fragment>
      )
    }
  }
  
  
  class QueryRow extends React.Component {
    render() {
      return (
        <tr>
            <th scope="row">{this.props.id}</th>
            <td>{this.props.func}</td>
            <td className="text-right">delete</td>
        </tr>
      )
    }
  }
  