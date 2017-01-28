import React from 'react';
import { FormControl, FormGroup, ControlLabel, HelpBlock } from 'react-bootstrap'

class UserForms {
    constructor(component) {
        this.component = component;
        this.validators = {};
    }

    control(label, name, type, validate) {
        let controlState= this.component.state[name];
        if(! controlState) {
            controlState = {validation: this.validationResult(), value: ''};
            this.component.state[name] = controlState;
        }
        this.validators[name] = validate;
        return ( 
            <FormGroup controlId={'control-' + name} validationState={controlState.validation.state}>
                <ControlLabel>{label}</ControlLabel>
                <FormControl name={name} type={type} value={controlState.value} onChange={(e) => this.handleChange(name, e.target) }/>
                { !!controlState.validation.message && <HelpBlock>{controlState.validation.message}</HelpBlock> }
            </FormGroup>
        );
    }

    handleChange(name, target) {
        this.validate(name, target.value);
    }

    validate(name, value) {
        let validator = this.validators[name];
        let validation = null;
        if(validator) {
            validation = validator(value, name);
        }
        if(! validation)
            validation = this.validationResult();
        let state = {[name]: { value: value, validation: validation }};
        this.component.setState(state);
        return validation;
    }

    validateAll() {
        return Object.keys(this.validators).map( (name) => {
            let result = this.validate(name, this.component.state[name].value);
            return { name: name, validation: result};
        });
    }

    
    validationResult(state, message) {
        return {message: message ? message : null, state: state ? state : null};
    }

    setManualState(control, state) {
        let componentState = this.component.state;
        componentState[control].validation.state = state;
        this.component.setState(componentState);
    }
}

export default UserForms;
