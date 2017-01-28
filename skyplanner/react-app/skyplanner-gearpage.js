import React from 'react'
import { Table } from 'react-bootstrap'
import Ajax from './ajax'
import URLs from './urls'

class TelescopeRow extends React.Component {
    render() {
        return (
            <tr>
                <td>{this.props.telescope.name}</td>
                <td>{this.props.telescope.focal_length}</td>
                <td>{this.props.telescope.diameter}</td>
            </tr>
        );
    }
}

class TelescopesFrame extends React.Component {
    render() {
        return (
            <div>
                <h3>Telescopes</h3>
                <Table bordered condensed hover>
                    <thead>
                        <tr>
                            <th>Name</th>
                            <th>Focal Length</th>
                            <th>Diameter</th>
                        </tr>
                    </thead>
                    <tbody>
                        {this.rows()}
                    </tbody>
                </Table>
            </div>
        );
    }

    rows() {
        return this.props.telescopes.map( (t) => <TelescopeRow key={t.id} telescope={t} /> );
    }
}

class SkyPlannerGearPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = { telescopes: [] };
    }

    componentDidMount() {
        Ajax.fetch(URLs.buildAuthPath('/api/telescopes'))
            .then(Ajax.decode_json({
                is_success: (r) => r.status == 200,
                success: this.setTelescopes.bind(this)
        }));
    }

    render() {
        return (
            <div className='container'>
                <TelescopesFrame telescopes={this.state.telescopes}/>
            </div>
        );
    }

    setTelescopes(telescopes) {
        this.setState({telescopes: telescopes});
    }
}

export default SkyPlannerGearPage;
