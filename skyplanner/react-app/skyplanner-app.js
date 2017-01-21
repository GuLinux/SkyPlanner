import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';
import { RouteTransition } from 'react-router-transition';
import AuthManager from './auth-manager';



class SkyPlannerApp extends React.Component {
    constructor(props) {
        super(props);
        this.state = {user: null};
    }

    componentDidMount() {
        AuthManager.register(this);
    }

    componentWillUnmount() {
        AuthManager.unregister(this);
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
        return this.props.route.navs[state];
    }

}
export default SkyPlannerApp;
