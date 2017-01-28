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

        this.validators = {
            name: (v) => !!v,
            focal_length: (v) => v > 0,
            diameter: (v) => v>0
        };
    }

    render() {
        return (
            <tr>
                {this.formGroup('name', 'Telescope Name', 'text')}
                {this.formGroup('focal_length', 'Focal Length', 'number')}
                {this.formGroup('diameter', 'Diameter', 'number')}
                <td>
                    <ButtonGroup>
                        <Button bsSize="xsmall" bsStyle="primary" onClick={this.submit.bind(this)}>save</Button>
                        { 'telescope' in this.props && <Button bsSize="xsmall" onClick={this.props.onCancel}>cancel</Button> }
                    </ButtonGroup>
                </td>
            </tr>
        );

    }
    formGroup(name, placeholder, type) {
        return (
            <td>
                <FormGroup controlId={name} bsSize="sm" className="form-group-table" validationState={this.state[name + '_error']}>
                    <FormControl type={type} placeholder={placeholder} value={this.state[name]} onChange={ (e) => this.validate(name, e.target.value)} />
                </FormGroup>
            </td>
        );
    }

    validate(name, value) {
        let isValid = this.validators[name](value);
        this.setState({[name]: value, [name + '_error']: isValid ? 'success' : 'error'});
        return isValid;
    }

    submit() {
        if(['name', 'diameter', 'focal_length'].map( (v) => this.validate(v, this.state[v])).some((v) => !v))
            return;
        Ajax.send_json(URLs.buildAuthPath('/api/telescopes'), this.state, 'PUT')
            .then(Ajax.decode_json({
                is_success: (r) => r.status == 201,
                success: this.props.onSuccess,
                failure: this.onFailure
            })
        )
    }

    onFailure(a, b) {
        console.log('error creating telescope');
        console.log(a)
        console.log(b)
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
                        <TelescopeEditRow onSuccess={this.props.onChange}/>
                    </tbody>
                </Table>
            </div>
        );
    }

    rows() {
        return this.props.telescopes.map( (t) => {
            if( this.state.edit_telescope && this.state.edit_telescope.id == t.id)
                return <TelescopeEditRow telescope={t} key={t.id} onCancel={ () => this.setState({edit_telescope: null}) } />;
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
        this.loadTelescopes()
    }

    render() {
        return (
            <div className='container'>
                <TelescopesTable telescopes={this.state.telescopes} onChange={this.loadTelescopes.bind(this)}/>
            </div>
        );
    }

    setTelescopes(telescopes) {
        this.setState({telescopes: telescopes});
    }

    loadTelescopes() {
         Ajax.fetch(URLs.buildAuthPath('/api/telescopes'))
            .then(Ajax.decode_json({
                is_success: (r) => r.status == 200,
                success: this.setTelescopes.bind(this)
        }));
   }
}

export default SkyPlannerGearPage;
