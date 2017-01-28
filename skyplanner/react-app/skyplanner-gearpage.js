import React from 'react'
import { Table, Button, ButtonGroup, FormControl, FormGroup } from 'react-bootstrap'
import Ajax from './ajax'
import URLs from './urls'

class TelescopeRow extends React.Component {
    render() {
        return (
            <tr>
                <td>{this.props.telescope.name}</td>
                <td>{this.props.telescope.focal_length}</td>
                <td>{this.props.telescope.diameter}</td>
                <td><ButtonGroup>
                    <Button bsSize="xsmall" onClick={this.props.onEdit}>edit</Button>
                    <Button bsStyle="danger" bsSize="xsmall">remove</Button>
                </ButtonGroup></td>
            </tr>
        );
    }
}

class TelescopeEditRow extends React.Component {
    constructor(props) {
        super(props);
        this.state = {name: "", focal_length: "" , diameter: ""};
        if('telescope' in props) {
            Object.assign(this.state, props.telescope);
        }
    }

    render() {
        return (
            <tr>
                {this.formGroup('name', 'Telescope Name', 'text')}
                {this.formGroup('focal_length', 'Focal Length', 'number')}
                {this.formGroup('diameter', 'Diameter', 'number')}
                <td><Button bsSize="xsmall" bsStyle="primary">save</Button></td>
            </tr>
        );

    }
    formGroup(name, placeholder, type) {
        return (
            <td>
                <FormGroup controlId={name} bsSize="sm" className="form-group-table">
                    <FormControl type={type} placeholder={placeholder} value={this.state[name]} />
                </FormGroup>
            </td>
        );
    }

}

class TelescopesTable extends React.Component {
    constructor(props) {
        super(props);
        this.state = { edit_telescope: null };
    }

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
                            <th />
                        </tr>
                    </thead>
                    <tbody>
                        {this.rows()}
                        <TelescopeEditRow />
                    </tbody>
                </Table>
            </div>
        );
    }

    rows() {
        return this.props.telescopes.map( (t) => {
            if( this.state.edit_telescope && this.state.edit_telescope.id == t.id)
                return <TelescopeEditRow telescope={t} />;
            return <TelescopeRow key={t.id} telescope={t} onEdit={ () => this.setState({edit_telescope: t}) } />
        });
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
                <TelescopesTable telescopes={this.state.telescopes}/>
            </div>
        );
    }

    setTelescopes(telescopes) {
        this.setState({telescopes: telescopes});
    }
}

export default SkyPlannerGearPage;
