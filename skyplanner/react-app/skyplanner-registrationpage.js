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
            <form className='container' onSubmit={this.register}>
                {this.forms.control('Username', 'username', 'text', this.validateUsername.bind(this))}
                {this.forms.control('Password', 'password', 'password', this.validatePassword.bind(this))}
                {this.forms.control('Confirm Password', 'confirm-password', 'password', this.validatePasswordConfirm.bind(this))}
                <Button type='submit'>Register</Button>
            </form>
        );
    }

    validateUsername(value) {
        if(value.length < 4)
            return this.forms.validationResult('error', 'Username must beat least 4 characters');
    }

    validatePassword(value) {
    }

    validatePasswordConfirm(value) {
    }
}

export default SkyPlannerRegistrationPage;
