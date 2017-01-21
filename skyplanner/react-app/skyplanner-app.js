import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';
import { RouteTransition } from 'react-router-transition';
import AuthManager from './auth-manager';
import { NotificationManager } from 'react-notifications';
import URLs from './urls';


class SkyPlannerApp extends React.Component {
    constructor(props) {
        super(props);
        console.log(props);
    }

    componentDidMount() {
        AuthManager.register(this);
        let token = AuthManager.token();
        if(token != null) {
            Ajax.fetch( URLs.buildAuthPath('/api/users/get'))
                .then(Ajax.decode_json({
                    success: (j) => { Object.assign(j, {token: token}); AuthManager.login(j); } 
            }));
        }
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
                    <div className='container-fluid'>{this.props.children}</div>
                </RouteTransition>
            </div>
        ); 
    }

    loginChanged(user) {
        if(!user) {
            NotificationManager.success('User logged out correctly', 'Logout', 5000);
            props.router.replace('/');
        }
    }
}
export default SkyPlannerApp;
