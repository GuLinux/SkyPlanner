import React from 'react';
import { FormControl, FormGroup, ControlLabel, HelpBlock } from 'react-bootstrap'

class UserForms {
    constructor(component) {
        this.component = component;
    }

    control(label, name, type, validate) {
        let controlState= this.component.state[name];
        if(! controlState) {
            controlState = {validation: this.validationResult(), value: ''};
        }
        return ( 
            <FormGroup controlId={'control-' + name} validationState={controlState.validation.state}>
                <ControlLabel>{label}</ControlLabel>
                <FormControl name={name} type={type} value={controlState.value} onChange={(e) => this.handleChange(name, e.target, validate) }/>
                { !!controlState.validation.message && <HelpBlock>{controlState.validation.message}</HelpBlock> }
            </FormGroup>
        );
    }

    handleChange(name, target, validate) {
        let validation = null;
        if(validate) {
            validation = validate(target.value, target, name);
        }
        if(! validation)
            validation = this.validationResult();
        this.component.setState({[name]: { value: target.value, validation: validation }});
    }
    
    validationResult(state, message) {
        return {message: message ? message : null, state: state ? state : null};
    }
}

export default UserForms;
