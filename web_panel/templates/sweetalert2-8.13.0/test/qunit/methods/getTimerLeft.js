import { Swal } from '../helpers.js'

QUnit.test('getTimerLeft() method', (assert) => {
  Swal.fire({
    timer: 1000
  })
  const timerLeft = Swal.getTimerLeft()
  assert.ok(timerLeft > 0)
  assert.ok(timerLeft <= 1000)
})

QUnit.test('getTimerLeft() should return null if popup does not have timer', (assert) => {
  Swal.fire({
    timer: 1000
  })
  Swal.fire('I do not have timer, I should reset timer')

  const timerLeft = Swal.getTimerLeft()
  assert.equal(timerLeft, null)
})
