import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';
import { RouteTransition } from 'react-router-transition';
import LoginDispatcher from './login-dispatcher';



class SkyPlannerApp extends React.Component {
    constructor(props) {
        super(props);
        this.state = {user: undefined};
    }

    componentDidMount() {
        LoginDispatcher.register(this);
    }

    componentWillUnmount() {
        LoginDispatcher.unregister(this);
    }

    render() {
        return (
            <div>
                <SkyPlannerNavigation navs={this.navs()}/>
                <RouteTransition pathname={this.props.location.pathname} 
                    atEnter={{ opacity: 0 }}
                    atLeave={{ opacity: 0 }}
                    atActive={{ opacity: 1 }}>
                    <div className='container-fluid'>{this.props.children}</div>
                </RouteTransition>
            </div>
        ); 
    }

    setUser(user) {
        this.setState({user: user});
    }

    navs() {
        var state = this.state.user ? 'loggedIn' : 'loggedOut';
        return this.props.navs[state];
    }

}
export default SkyPlannerApp;
