import React from 'react';
import Ajax from './ajax';
import { NotificationManager } from 'react-notifications';
import AuthManager from './auth-manager';
import URLs from './urls';
import { FormControl, FormGroup, ControlLabel, Button, Checkbox} from 'react-bootstrap'
import UserForms from './user-forms'

class SkyPlannerLoginPage extends React.Component {
    constructor(props) {
        super(props);
        this.login = this.login.bind(this);
        this.forms = new UserForms(this);
        this.state = {};
    }
    render() {
       return (
            <form className='container' onSubmit={this.login}>
           
                {this.forms.control('Username', 'username', 'text', this.validateControl.bind(this))}
                {this.forms.control('Password', 'password', 'password', this.validateControl.bind(this))}
                <FormGroup>
                    <Checkbox name='remember-me' inline checked={this.state.remember} onChange={(e) => {this.setState({remember: !this.state.remember }) } }>Remember me</Checkbox>
                </FormGroup>
                <Button type='submit'>Login</Button>
            </form>
       )
    }

    login(e) {
        e.preventDefault();
        if( this.forms.validateAll().some( (v) => v.validation.state == 'error' ) ) {
            NotificationManager.warning('Please fill the required fields', 'Login', 5000);
            return;
        }
        Ajax.send_json('/api/users/login', {username: this.state.username.value, password: this.state.password.value}, 'POST')
            .then(Ajax.decode_json({
                is_success: (r) => r.status == 200,
                success: this.loginSuccess.bind(this),
                failure: this.loginFailure.bind(this)
        }));
    }

    validateControl(value, name) {
        if(! value) {
            return this.forms.validationResult('error', (name == 'username' ? 'Username' : 'Password') + ' must not be empty');
        }
    }

    loginFailure(json, response) {
        if(response.status == 401) {
            this.forms.setManualState('username', 'error');
            this.forms.setManualState('password', 'error');
            NotificationManager.warning('Invalid username or password', 'Login Error', 5000);
        } else {
            NotificationManager.warning(json.reason, 'Login Error', 5000);
        }
    }

    loginSuccess(json) {
        NotificationManager.success('User ' + json.username + ' correctly logged in', 'Login', 5000);
        AuthManager.login(Object.assign(json.user, {token: json.token}), this.state.remember);
        var nextLocation = URLs.root.path;
        try {
            nextLocation = this.props.location.state.nextPathname;
        } catch(TypeError) {}
        this.props.router.replace(nextLocation);
    }
}
export default SkyPlannerLoginPage;
