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
        this.errors = {
            username_already_existing: {msg: 'username already exists.', fields: ['username']},
            username_too_short: {msg: 'username must be at last 4 characters.', fields: ['username']},
            password_too_short: {msg: 'password must be at least 8 characters.', fields: ['password']},
            passwords_not_matching: {msg: 'passwords do not match', fields: ['confirm-password']},
            generic_registration_error: {msg: 'generic registration error', fields: []}
        }
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
            return this.forms.validationResult('error', this.errors.username_too_short.msg);
    }

    validatePassword(value) {
        if(value.length < 8)
            return this.forms.validationResult('error', this.errors.password_too_short.msg);
    }

    validatePasswordConfirm(value) {
        if(value != this.state.password.value)
            return this.forms.validationResult('error', this.errors.passwords_not_matching.msg);
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
        let error = json.reason in this.errors ? this.errors[json.reason] : this.errors.generic_registration_error;
        NotificationManager.warning(error.msg, 'Registration Error', 5000);
        error.fields.forEach( (f) => this.forms.setManualState(f, 'error', error.msg));
    }

    registrationSuccess(json) {
        NotificationManager.success('Registration successfully completed', 'Registration', 4999);
        this.props.router.push(URLs.login.path);
    }
}

export default SkyPlannerRegistrationPage;
