import React from 'react';
import Ajax from './ajax';
import { NotificationManager } from 'react-notifications';

import { FormControl, FormGroup, ControlLabel, Button } from 'react-bootstrap'
class SkyPlannerLoginPage extends React.Component {
    constructor(props) {
        super(props);
        this.handleChange = this.handleChange.bind(this);
        this.login = this.login.bind(this);
        this.state = {username: '', password: ''};
    }
    render() {
       return (
            <form className='container' onSubmit={this.login}>
                <FormGroup controlId='login-username' validationState={this.state.username_state}>
                    <ControlLabel>Username</ControlLabel>
                    <FormControl name='username' type='text' value={this.state.username} onChange={this.handleChange}/>
                </FormGroup>
                <FormGroup controlId='login-password' validationState={this.state.password_state}>
                    <ControlLabel>Password</ControlLabel>
                    <FormControl name='password' type='password' value={this.state.password} onChange={this.handleChange}/>
                </FormGroup>
                <Button type='submit'>Login</Button>
            </form>
       )
    }

    login(e) {
        e.preventDefault();

        if( ['username', 'password'].map( (v) => this.validate(v, this.state[v])  ).some( (v) => !v )  ) {
            NotificationManager.warning('Please fill the required fields', 'Login', 5000);
            return;
        }
        Ajax.send_json('/api/users/login', this.state, 'POST')
            .then(Ajax.decode_json({
                is_success: this.checkLoginSuccess.bind(this),
                success: this.loginSuccess.bind(this)
        }));
    }

    handleChange(e) {
        this.validate(e.target.name, e.target.value);
    }

    validate(controlName, value) {
        this.setState({[controlName]: value, [controlName + '_state']: value.length > 0 ? undefined : 'error' });
        return value.length > 0;
    }

    checkLoginSuccess(response) {
        if(response.status == 401) {
            this.setState({username_state: 'error', password_state: 'error'});
            NotificationManager.warning('Invalid username or password', 'Login Error', 5000);
        } 
        return response.status == 200;
    }

    loginSuccess(json) {
        NotificationManager.success('User ' + json.username + ' correctly logged in', 'Login', 5000);
        this.props.onLogin(Object.assign(json.user, {token: json.token}));
    }
}
export default SkyPlannerLoginPage;
