import React from 'react'

class Telescopes extends React.Component {
    render() {
        return <p>Telescopes List</p>;
    }
}

class TelescopesFrame extends React.Component {
    render() {
        return (
            <div>
                <h2>Telescopes</h2>
                <Telescopes />
            </div>
        );
    }
}

class SkyPlannerGearPage extends React.Component {
    render() {
        return (
            <div className='container'>
                <TelescopesFrame />
            </div>
        );
    }
}

export default SkyPlannerGearPage;
