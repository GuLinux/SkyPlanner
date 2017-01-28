import React from 'react';
import Ajax from './ajax';
import { NotificationManager } from 'react-notifications';
import AuthManager from './auth-manager';
import URLs from './urls';
import { FormControl, FormGroup, ControlLabel, Button, Checkbox} from 'react-bootstrap'
import UserForms from './user-forms'


class SkyPlannerRegistrationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {username: '', password: '', remember: false};
        this.forms = new UserForms(this);
    }

    render() {
        return (
            <form className='container' onSubmit={this.register.bind(this)}>
                {this.forms.control('Username', 'username', 'text', this.validateUsername.bind(this))}
                {this.forms.control('Password', 'password', 'password', this.validatePassword.bind(this))}
                {this.forms.control('Confirm Password', 'confirm-password', 'password', this.validatePasswordConfirm.bind(this))}
                <Button type='submit'>Register</Button>
            </form>
        );
    }

    validateUsername(value) {
        if(value.length < 4)
            return this.forms.validationResult('error', 'Username must be at least 4 characters');
    }

    validatePassword(value) {
        if(value.length < 8)
            return this.forms.validationResult('error', 'Password must be at least 8 characters');
    }

    validatePasswordConfirm(value) {
        if(value != this.state.password.value)
            return this.forms.validationResult('error', 'Passwords do not match');
    }

    register(e) {
        e.preventDefault();
        if( this.forms.validateAll().some( (v) => v.validation.state == 'error' ) ) {
            NotificationManager.warning('Please check the required fields', 'Registration', 5000);
            return;
        }
        Ajax.send_json('/api/users/create', {username: this.state.username.value, password: this.state.password.value}, 'PUT')
            .then(Ajax.decode_json({
                is_success: (r) => r.status == 201,
                success: this.registrationSuccess.bind(this),
                failure: this.registrationFailure.bind(this)
        }));
    }

    registrationFailure(json, response) {
        NotificationManager.warning('Error on registration: ' + json.reason, 'Registration Error', 5000);
    }

    registrationSuccess(json) {
        NotificationManager.success('Registration successfully completed', 'Registration', 5000);
        this.props.router.push(URLs.login.path);
    }
}

export default SkyPlannerRegistrationPage;
