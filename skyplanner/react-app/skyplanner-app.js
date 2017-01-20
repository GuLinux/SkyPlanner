import React from 'react';
import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerContentManager from './skyplanner-content-manager';
import SkyPlannerHomePage from './skyplanner-homepage';
import SkyPlannerLoginPage from './skyplanner-loginpage';


class SkyPlannerApp extends React.Component {
    constructor(props) {
        super(props);
        this.state = {page: 'home'};
    }

    render() {
        return (
            <div>
                <SkyPlannerNavigation navs={this.navs()}/>
                <SkyPlannerContentManager>{this.pages()}</SkyPlannerContentManager>
            </div>
        ); 
    }

    navs() {
        return [{key: 'home', display: 'Home'}, {key: 'login', display: 'Login'}];
    }

    pages() {
        return [
            <SkyPlannerHomePage key='home' />,
            <SkyPlannerLoginPage key='login' />
        ];
    }

    onNavClick(e) {
        this.setState({page: 'x'});
    }
}
export default SkyPlannerApp;
