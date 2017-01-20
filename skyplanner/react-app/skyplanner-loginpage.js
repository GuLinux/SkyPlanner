import React from 'react';
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
                <FormGroup controlId='login-username'>
                    <ControlLabel>Username</ControlLabel>
                    <FormControl name='username' type='text' value={this.state.username} onChange={this.handleChange}/>
                </FormGroup>
                <FormGroup controlId='login-password'>
                    <ControlLabel>Password</ControlLabel>
                    <FormControl name='password' type='password' value={this.state.password} onChange={this.handleChange}/>
                </FormGroup>
                <Button type='submit'>Login</Button>
            </form>
       )
    }

    login() {
        console.log(this);
    }

    handleChange(e) {
        this.setState({[e.target.name]: e.target.value})
    }
}
export default SkyPlannerLoginPage;
