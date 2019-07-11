const { Swal, isVisible, TIMEOUT } = require('./helpers')
import defaultInputValidators from '../../src/utils/defaultInputValidators.js'

QUnit.test('input.checkValidity()', (assert) => {
  const done = assert.async()

  Swal.fire({
    input: 'tel',
    inputAttributes: {
      pattern: '[0-9]{3}-[0-9]{3}-[0-9]{4}'
    },
    validationMessage: 'Invalid phone number'
  }).then(result => {
    assert.equal(result.value, '123-456-7890')
    done()
  })

  Swal.getInput().value = 'blah-blah'
  Swal.clickConfirm()
  setTimeout(() => {
    assert.ok(isVisible(Swal.getValidationMessage()))
    assert.equal(Swal.getValidationMessage().textContent, 'Invalid phone number')

    Swal.getInput().value = '123-456-7890'
    Swal.clickConfirm()
  }, TIMEOUT)
})

QUnit.test('default URL validator', (assert) => {
  const done = assert.async(3)

  defaultInputValidators.url('https://google.com').then(() => {
    done()
  })

  defaultInputValidators.url('http://foo.localhost/').then(() => {
    done()
  })

  defaultInputValidators.url('invalid url').then(data => {
    assert.equal(data, 'Invalid URL')
    done()
  })
})
