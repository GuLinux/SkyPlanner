import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';
import { RouteTransition } from 'react-router-transition';



class SkyPlannerApp extends React.Component {

    render() {
        return (
            <div>
                <SkyPlannerNavigation navs={this.props.route.navs}/>
                <RouteTransition pathname={this.props.location.pathname} 
                    atEnter={{ opacity: 0 }}
                    atLeave={{ opacity: 0 }}
                    atActive={{ opacity: 1 }}>
                    <div className='container-fluid'>{this.props.children}</div>
                </RouteTransition>
            </div>
        ); 
    }

}
export default SkyPlannerApp;
