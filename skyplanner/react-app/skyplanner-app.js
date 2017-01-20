import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';


class SkyPlannerApp extends React.Component {

    render() {
        return (
            <div>
                <SkyPlannerNavigation navs={this.props.route.navs}/>
                <div className='container-fluid'>{this.props.children}</div>
            </div>
        ); 
    }

}
export default SkyPlannerApp;
