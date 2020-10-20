import axios from 'axios'
const base_url = "http://localhost:8888"
axios.all([axios.get(`${base_url}/`), axios.get(`${base_url}/`), axios.get(`${base_url}/`), axios.get(`${base_url}/`), axios.get(`${base_url}/`)])
    .then(axios.spread((response1, response2, response3, response4, response5) => {
        console.log(
            'Response 1', response1.data.slice(0, 30),
            'Response 2', response2.data.slice(0, 30),
            'Response 3', response3.data.slice(0, 30),
            'Response 4', response4.data.slice(0, 30),
            'Response 5', response5.data.slice(0, 30),
        )
    }));