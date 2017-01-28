import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';
import { RouteTransition } from 'react-router-transition';
import AuthManager from './auth-manager';
import { NotificationManager } from 'react-notifications';
import URLs from './urls';


class SkyPlannerApp extends React.Component {

    componentDidMount() {
        AuthManager.register(this);
    }

    componentWillUnmount() {
        AuthManager.unregister(this);
    }

    render() {
        return (
            <div>
                <SkyPlannerNavigation />
                <RouteTransition pathname={this.props.location.pathname} 
                    atEnter={{ opacity: 0 }}
                    atLeave={{ opacity: 0 }}
                    atActive={{ opacity: 1 }}>
                    {this.props.children}
                </RouteTransition>
            </div>
        ); 
    }

    loginChanged(user) {
        if(!user) {
            NotificationManager.success('User logged out correctly', 'Logout', 5000);
            this.props.router.replace('/');
        }
    }
}
export default SkyPlannerApp;
